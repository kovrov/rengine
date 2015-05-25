#include "rengine.h"
#include "test.h"

#include <vector>

using namespace rengine;
using namespace std;

class Animation
{
public:

    enum Direction { // uses 2-bit field
        Normal,
        Reverse,
        Alternate,
        AlternateReverse
    };

    enum ActiveDirection {
        Forwards,
        Backwards
    };

    Animation()
      : m_iterations(1)
      , m_currentIteration(0)
      , m_duration(0.0)
      , m_startTime(-1.0)
      , m_running(false)
      , m_direction(Normal)
    {
    }

    virtual ~Animation()
    {
    }

    void tick(double time, ActiveDirection direction = Forwards) {
        cout << "tick: time=" << int(time * 10000)/10000.0
             << "; direction=" << (direction == Forwards ? "Forward" : "Backward")
             << endl;
        assert(isRunning());
        assert(time >= 0);

        if (time >= m_duration) {
            int iteration = (int) (time / m_duration);
            double timeInThisIteration = fmod(time, m_duration);

            if (iteration >= m_iterations) {
                // finish the animation at its end value..
                cout << " - reached end, stopping" << endl;
                apply_helper(m_duration, direction);
                stop();
                return;
            } else {
                if (m_currentIteration != iteration) {
                    cout << " - new iteration: " << m_currentIteration << "->" << iteration << endl;
                    iterationChanged(iteration, direction);
                    m_currentIteration = iteration;
                }
                apply_helper(timeInThisIteration, direction);
            }
        } else {
            apply_helper(time, direction);
        }
    }

    void apply_helper(double time, ActiveDirection activeDirection) {
        if (m_direction == Reverse
            || (m_direction == Alternate && ((m_currentIteration % 2) == 1))
            || (m_direction == AlternateReverse && (m_currentIteration % 2 == 0))) {
            apply(m_duration - time, activeDirection == Forwards ? Backwards : Forwards);
        } else {
            apply(time, activeDirection);
        }
    }

    /*!
        The animation implements this function to handle the animation
        update for the given \a time. The time is guaranteed to be bounded
        between 0 <= time <= duration.
     */
    virtual void apply(double time, ActiveDirection activeDirection) = 0;

    /*!
        Reimplement this function to be notified whenever an animation
        has changed loops. The iteration is bounded to 0 <= iteration <= iterations;
     */
    virtual void iterationChanged(int iteration, ActiveDirection activeDirection) { }

    virtual void stop() {
        setRunning(false);
    }

     double duration() const { return m_duration; }
     void setDuration(double dur) {
         assert(!isRunning());
         m_duration = dur;
     }

     int iterations() { return m_iterations; }
     void setIterations(int iterations) {
         assert(!isRunning());
         m_iterations = iterations;
    }

     bool isRunning() const { return m_running; }
     void setRunning(bool r) {
         assert(r != m_running);
         m_running = r;
         m_currentIteration = 0;
     }

     Direction direction() const { return (Direction) m_direction; }
     void setDirection(Direction d) { m_direction = d; }

     /*!
         Used for bookeeping purposes within the animation system. Should
         not be called by outside parties.
      */
     double startTime() const { return m_startTime; }
     void setStartTime(double t) {
         assert(!isRunning());
         m_startTime = t;
     }

private:
    int m_iterations;
    int m_currentIteration;
    double m_duration;
    double m_startTime;

    unsigned m_running : 1;
    unsigned m_direction : 2;
};



class GroupAnimation : public Animation
{
public:

    ~GroupAnimation() {
        for (auto a : m_children)
            delete a;
    }

    void append(Animation *a) {
        assert(!isRunning());
        assert(find(m_children.begin(), m_children.end(), a) == m_children.end());
        assert(a->direction() == Normal); // We care about direction for toplevel animations

        m_children.push_back(a);
        updateDuration();
    }

    void remove(Animation *a) {
        assert(!isRunning());

        auto entry = find(m_children.begin(), m_children.end(), a);
        assert(entry != m_children.end());

        m_children.erase(entry);
        updateDuration();
    }

    void stop() {
        for (auto a : m_children)
            if (a->isRunning())
                a->stop();
        Animation::stop();
    }

protected:
    virtual void updateDuration() = 0;
    vector<Animation *> m_children;
};



class SequentialAnimation : public GroupAnimation
{
public:
    SequentialAnimation()
      : m_current(-1)
    {
    }

    void iterationChanged(int iteration, ActiveDirection activeDirection) {
        if (activeDirection == Forwards) {
            for (int i = m_current; i<m_children.size(); ++i)
                applyAndStop(m_children.at(i), Forwards);
        } else {
            for (int i = m_current-1; i>=0; --i)
                applyAndStop(m_children.at(i), Backwards);
        }

    }

    void apply(double time, ActiveDirection activeDirection) {
        cout << " - sequential::apply: time=" << int(time * 10000) / 10000
             << "; " << (activeDirection == Forwards ? "Forwards" : "Backwards")
             << endl;

        int current = -1;

        double start = 0;
        for (int i=0; i<m_children.size(); ++i) {
            Animation *a = m_children.at(i);
            double end = start + a->iterations() * a->duration();
            if (time >= start && time < end) {
                current = i;
                break;
            }
            start = end;
        }

        // We're out of bounds, so pick the last one and let the system advance...
        if (current == -1 && time >= start) {
            cout << "   - out of bounds, using last" << endl;
            current = m_children.size() - 1;
        }
        assert(current >= 0);

        if (m_current != current) {
            if (m_current >= 0) {
                // We need to 'finish' all the animations in between the old
                // one and the new one, so they end on their expected target
                // value.
                // In the case where we cross to the next iteration is already
                // handled in tick().
                if (activeDirection == Forwards) {
                    for (int i = m_current; i<current; ++i)
                        applyAndStop(m_children.at(i), Forwards);
                } else {
                    for (int i = m_current-1; i>=current; --i)
                        applyAndStop(m_children.at(i), Backwards);
                }
            }
            m_current = current;
            Animation *a = m_children.at(m_current);
            a->setStartTime(time);
            a->setRunning(true);
        }

        m_children.at(m_current)->tick(time - start, activeDirection);
    }

protected:
    void updateDuration() {
        double total = 0;
        for (auto a : m_children)
            total += a->iterations() * a->duration();
        setDuration(total);
    }

private:
    static void applyAndStop(Animation *a, ActiveDirection activeDirection) {
        if (!a->isRunning())
            a->setRunning(true);
        a->apply(activeDirection == Forwards ? a->duration() : 0, activeDirection);
        a->setRunning(false);
    }

    int m_current;
};



class ValueTrackerAnimation : public Animation
{
public:
    void apply(double t, ActiveDirection adir) {
        appliedTimes.push_back(t);
    }
    void iterationChanged(int it, ActiveDirection adir) {
        itertionsChanged.push_back(it);
    }

    void run(double duration, int iterations, double increment)
    {
        setDuration(duration);
        setIterations(iterations);
        setRunning(true);
        appliedTimes = vector<double>();
        itertionsChanged = vector<int>();
        double t = 0;
        while (isRunning()) {
            tick(t);
            t += increment;
        }
    }

    vector<double> appliedTimes;
    vector<int> itertionsChanged;
};

void tst_animationsystem_runsingle()
{
    ValueTrackerAnimation anim;

    // Normal forward animation
    anim.run(10, 5, 1);
    for (int i=0; i<50; ++i)
        check_equal(anim.appliedTimes.at(i), i % 10);
    check_equal(anim.appliedTimes.back(), 10);
    for (int i=0; i<4; ++i)
        check_equal(anim.itertionsChanged.at(i), i + 1);
    cout << __FUNCTION__ << ": direction(Normal): ok" << endl;

    // Reverse
    anim.setDirection(Animation::Reverse);
    anim.run(10, 5, 1);
    for (int i=0; i<50; ++i)
        check_equal(anim.appliedTimes.at(i), 10 - i % 10);
    check_equal(anim.appliedTimes.back(), 0);
    for (int i=0; i<4; ++i)
        check_equal(anim.itertionsChanged.at(i), i + 1);
    cout << __FUNCTION__ << ": direction(Reverse): ok" << endl;

    // Alternate
    anim.setDirection(Animation::Alternate);
    anim.run(10, 5, 1);
    for (int i=0; i<=50; ++i) {
        int ii = i % 10;
        if ((i / 10) % 2 == 1)
            ii = 10 - ii;
        check_equal(anim.appliedTimes.at(i), ii);
    }
    cout << __FUNCTION__ << ": direction(Alternate): ok" << endl;

    // AlternateReverse
    anim.setDirection(Animation::AlternateReverse);
    anim.run(10, 5, 1);
    for (int i=0; i<=50; ++i) {
        int ii = i % 10;
        if ((i / 10) % 2 == 0)
            ii = 10 - ii;
        check_equal(anim.appliedTimes.at(i), ii);
    }
    cout << __FUNCTION__ << ": direction(AlternateReverse): ok" << endl;


    cout << __FUNCTION__ << ": ok" << endl;
}

class CharAddAnimation : public Animation
{
public:
    CharAddAnimation(string *s, char c)
        : text(s)
        , ch(c)
    {
        setIterations(1);
        setDuration(1);
    }

    void apply(double time, ActiveDirection dir) {
        text->push_back(ch);
    }

    string *text;
    char ch;
};

void tst_animationsystem_sequential_run(const char *name,
                                        int iterations,
                                        Animation::Direction direction,
                                        const string &result)
{
    string s;

    CharAddAnimation *anim_a = new CharAddAnimation(&s, 'a');
    CharAddAnimation *anim_b = new CharAddAnimation(&s, 'b');
    CharAddAnimation *anim_c = new CharAddAnimation(&s, 'c');

    SequentialAnimation sequential;
    sequential.append(anim_a);
    sequential.append(anim_b);
    sequential.append(anim_c);

    double t = 0;
    sequential.setIterations(iterations);
    sequential.setDirection(direction);
    sequential.setRunning(true);
    while (sequential.isRunning()) {
        sequential.tick(t);
        t += 1.0;
    }
    check_equal(s, result);
    assert(!anim_a->isRunning());
    assert(!anim_b->isRunning());
    assert(!anim_c->isRunning());

    cout << __FUNCTION__ << ": " << name << ": ok" << endl;
}

void tst_animationsystem_sequential()
{
    tst_animationsystem_sequential_run("single", 1, Animation::Normal, "aabbcc");
    tst_animationsystem_sequential_run("dual", 2, Animation::Normal, "aabbccaabbcc");
    tst_animationsystem_sequential_run("triple", 3, Animation::Normal, "aabbccaabbccaabbcc");

    tst_animationsystem_sequential_run("single-rev", 1, Animation::Reverse, "ccbbaa");
    tst_animationsystem_sequential_run("dual-rev", 2, Animation::Reverse, "ccbbaaccbbaa");
    tst_animationsystem_sequential_run("triple-rev", 3, Animation::Reverse, "ccbbaaccbbaa");

    // tst_animationsystem_sequential_run("single-alt", 1, Animation::Alternate, "ccbbaa");
    // tst_animationsystem_sequential_run("dual-alt", 2, Animation::Alternate, "aabbccccbbaa");
    // tst_animationsystem_sequential_run("triple-alt", 3, Animation::Alternate, "aabbccccbbaaaabbcc");

    // tst_animationsystem_sequential_run("single-altrev", 1, Animation::AlternateReverse, "ccbbaa");
    // tst_animationsystem_sequential_run("dual-altrev", 2, Animation::AlternateReverse, "ccbbaaaabbcc");
    // tst_animationsystem_sequential_run("triple-altrev", 3, Animation::AlternateReverse, "ccbbaaaabbccccbbaa");

    cout << __FUNCTION__ << ": ok" << endl;
}

int main(int argc, char **argv)
{
    tst_animationsystem_runsingle();
    tst_animationsystem_sequential();
}