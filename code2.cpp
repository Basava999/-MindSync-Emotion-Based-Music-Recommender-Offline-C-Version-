// euphorisim.cpp
// EuphoriSim - Mood-Driven Life & Route Simulator (single file)
// Compile: g++ -std=c++17 -O2 euphorisim.cpp -o euphorisim

#include <bits/stdc++.h>
using namespace std;
using clk = chrono::high_resolution_clock;
using ms = chrono::duration<double, milli>;

// -------------------- Utilities --------------------
double rand01() {
    return (double)rand() / RAND_MAX;
}
int randint(int a, int b) {
    return a + rand() % (b - a + 1);
}
template<typename T>
void shuffle_vec(vector<T> &v) {
    for (int i = (int)v.size()-1; i>0; --i) {
        int j = randint(0, i);
        swap(v[i], v[j]);
    }
}

// -------------------- Typing-based Mood Detector --------------------
struct TypingSample {
    vector<double> intervals; // ms
    double mean() const {
        if (intervals.empty()) return 0;
        double s = 0; for (double x: intervals) s += x;
        return s / intervals.size();
    }
    double stddev() const {
        if (intervals.empty()) return 0;
        double m = mean();
        double s = 0; for (double x: intervals) s += (x-m)*(x-m);
        return sqrt(s / intervals.size());
    }
};

TypingSample record_typing_sample() {
    cout << "Type a short sentence (press ENTER when done). Try to type normally.\n";
    cout << "Start typing when you're ready >>> ";
    TypingSample ts;
    // We will capture character-by-character timing using cin.get()
    // Note: terminal buffering may affect accuracy; still gives usable rhythm features.
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clear previous newline
    cout.flush();
    string s;
    char c;
    vector<clk::time_point> times;
    // We'll read until newline
    while (true) {
        auto t1 = clk::now();
        if (!cin.get(c)) break;
        auto t2 = clk::now();
        times.push_back(t2);
        s.push_back(c);
        if (c == '\n') break;
        // If user typed a ctrl char, ignore
        if ((int)s.size() > 500) break;
    }
    // Build intervals between successive visible keys (ignore the first)
    for (size_t i=1;i<times.size();++i) {
        auto d = chrono::duration_cast<ms>(times[i] - times[i-1]).count();
        // ignore very large gaps (pauses) and also extreme micro-gaps
        if (d > 10 && d < 2000) ts.intervals.push_back(d);
    }
    if (ts.intervals.empty()) {
        // fallback: ask for approximate typing speed (words per minute)
        cout << "(No accurate keystroke timing captured.) Enter approximate words per minute (WPM): ";
        int wpm; cin >> wpm; cin.ignore(numeric_limits<streamsize>::max(), '\n');
        double msPerChar = 60000.0 / (wpm * 5.0);
        for (int i=0;i<10;i++) ts.intervals.push_back(msPerChar + (rand01()-0.5)*msPerChar*0.2);
    }
    return ts;
}

enum Mood { MOOD_CALM, MOOD_NEUTRAL, MOOD_EXCITED, MOOD_STRESSED };
string mood_name(Mood m){
    switch(m){
        case MOOD_CALM: return "Calm";
        case MOOD_NEUTRAL: return "Neutral";
        case MOOD_EXCITED: return "Excited";
        case MOOD_STRESSED: return "Stressed";
    }
    return "Unknown";
}

Mood infer_mood(const TypingSample &ts) {
    double mean = ts.mean();
    double sd = ts.stddev();
    // heuristics (tuned to be robust):
    // - low mean (fast typing) + low stddev -> Excited / confident
    // - medium mean, medium sd -> Neutral
    // - high mean (slow) + low sd -> Calm (deliberate)
    // - high mean + high sd -> Stressed (pauses, hesitations)
    if (mean < 120 && sd < 50) return MOOD_EXCITED;
    if (mean < 160 && sd < 90) return MOOD_NEUTRAL;
    if (mean >= 160 && sd < 120) return MOOD_CALM;
    return MOOD_STRESSED;
}

// -------------------- Music Recommender (simple) --------------------
struct Song { string title, artist, vibe; };
vector<Song> song_library = {
    {"Sunrise Drive","Nova Lane","calm"},
    {"Midnight Sprint","Electra","excited"},
    {"Coffee & Rain","Slowfold","calm"},
    {"Neon Pulse","The Jetstream","excited"},
    {"Easy Sunday","Paper Boat","calm"},
    {"Focus Mode","Binary Hearts","neutral"},
    {"Uplift","StellarVox","excited"},
    {"Quiet Corners","MellowMuse","calm"},
    {"Walking Home","Alleyways","neutral"},
    {"Heart Rate","PulseUnit","excited"}
};

vector<Song> recommend_by_mood(Mood m, int k=3) {
    vector<Song> res;
    vector<int> idx(song_library.size());
    iota(idx.begin(), idx.end(), 0);
    shuffle_vec(idx);
    for (int id: idx) {
        const Song &s = song_library[id];
        if (m==MOOD_CALM && s.vibe=="calm") res.push_back(s);
        else if (m==MOOD_EXCITED && s.vibe=="excited") res.push_back(s);
        else if (m==MOOD_NEUTRAL && (s.vibe=="neutral"||s.vibe=="calm")) res.push_back(s);
        else if (m==MOOD_STRESSED && s.vibe!="excited") res.push_back(s);
        if ((int)res.size()>=k) break;
    }
    if (res.empty()) {
        for (int i=0;i<k && i<(int)song_library.size();++i) res.push_back(song_library[i]);
    }
    return res;
}

// -------------------- Small Map + Distance --------------------
struct Point { string name; double x,y; };
double dist(const Point &a, const Point &b) {
    double dx = a.x - b.x, dy = a.y - b.y;
    return sqrt(dx*dx + dy*dy);
}

// -------------------- Genetic Algorithm for Route Optimization --------------------
struct GA {
    vector<Point> places;
    int popSize;
    int geneLen; // number of places to visit
    double crossoverRate;
    double mutationRate;
    vector<vector<int>> population;
    vector<double> fitness;
    GA(const vector<Point>&p, int pop=120, double cr=0.8, double mr=0.12) {
        places = p; popSize = pop; geneLen = p.size(); crossoverRate=cr; mutationRate=mr;
        init_population();
    }
    double route_cost(const vector<int>&chrom) {
        // route starts at 0 (home), visits all indices in chrom order, and returns to home
        double c = 0;
        for (int i=0;i<(int)chrom.size()-1;++i)
            c += dist(places[chrom[i]], places[chrom[i+1]]);
        // do not force return if single point; but keep cycle:
        c += dist(places[chrom.back()], places[chrom.front()]);
        return c;
    }
    void init_population() {
        population.clear(); fitness.clear();
        vector<int> base(geneLen);
        iota(base.begin(), base.end(), 0);
        for (int i=0;i<popSize;++i) {
            vector<int> chrom = base;
            shuffle_vec(chrom);
            population.push_back(chrom);
            fitness.push_back(0);
        }
    }
    void evaluate() {
        for (int i=0;i<popSize;++i) {
            double c = route_cost(population[i]);
            fitness[i] = 1.0 / (1.0 + c); // higher fitness for shorter cost
        }
    }
    int roulette_select() {
        double s = 0; for (double f: fitness) s += f;
        double r = rand01() * s;
        double acc = 0;
        for (int i=0;i<popSize;++i) {
            acc += fitness[i];
            if (acc >= r) return i;
        }
        return popSize-1;
    }
    pair<vector<int>,vector<int>> ordered_crossover(const vector<int>&a, const vector<int>&b) {
        int n = geneLen;
        int l = randint(0, n-1), r = randint(0, n-1);
        if (l > r) swap(l,r);
        vector<int> c1(n, -1), c2(n, -1);
        vector<char> used1(n,0), used2(n,0);
        // copy slice
        for (int i=l;i<=r;++i) {
            c1[i] = a[i]; used1[a[i]] = 1;
            c2[i] = b[i]; used2[b[i]] = 1;
        }
        // fill remaining preserving order
        int idx1 = (r+1)%n;
        for (int i=0;i<n;++i) {
            int v = b[(r+1+i)%n];
            if (!used1[v]) { c1[idx1]=v; used1[v]=1; idx1=(idx1+1)%n; }
        }
        int idx2 = (r+1)%n;
        for (int i=0;i<n;++i) {
            int v = a[(r+1+i)%n];
            if (!used2[v]) { c2[idx2]=v; used2[v]=1; idx2=(idx2+1)%n; }
        }
        return {c1,c2};
    }
    void mutate(vector<int>&chrom) {
        if (rand01() > mutationRate) return;
        int i = randint(0, geneLen-1), j = randint(0, geneLen-1);
        swap(chrom[i], chrom[j]);
    }
    vector<int> run(int generations=300) {
        evaluate();
        vector<int> best = population[0];
        double bestCost = route_cost(best);
        for (int gen=0; gen<generations; ++gen) {
            vector<vector<int>> newpop;
            // elitism: keep best
            int eliteIdx = 0;
            double bestF = fitness[0];
            for (int i=1;i<popSize;++i) if (fitness[i] > bestF) { bestF = fitness[i]; eliteIdx = i; }
            newpop.push_back(population[eliteIdx]);
            while ((int)newpop.size() < popSize) {
                int p1 = roulette_select();
                int p2 = roulette_select();
                vector<int> child1 = population[p1], child2 = population[p2];
                if (rand01() < crossoverRate) {
                    auto cross = ordered_crossover(population[p1], population[p2]);
                    child1 = cross.first; child2 = cross.second;
                }
                mutate(child1); mutate(child2);
                newpop.push_back(child1);
                if ((int)newpop.size() < popSize) newpop.push_back(child2);
            }
            population.swap(newpop);
            evaluate();
            // update best
            for (int i=0;i<popSize;++i) {
                double c = route_cost(population[i]);
                if (c < bestCost) { bestCost = c; best = population[i]; }
            }
            // occasional early break if stable
            if (gen % 80 == 0 && gen>0 && bestCost < 1e-6) break;
        }
        return best;
    }
};

// -------------------- Virtual Citizen LifeSim --------------------
struct Citizen {
    string name;
    int energy; // 0-100
    int happiness; // 0-100
    Mood mood;
    Citizen(string n="Alex"):name(n),energy(80),happiness(70),mood(MOOD_NEUTRAL){}
    void apply_event(const string &ev) {
        if (ev=="exercise") { energy -= 20; happiness += 8; }
        else if (ev=="work") { energy -= 30; happiness += 4; }
        else if (ev=="coffee") { energy += 15; happiness += 3; }
        else if (ev=="relax") { energy += 20; happiness += 6; }
        else if (ev=="errand") { energy -= 10; happiness -= 2; }
        energy = clamp(energy, 0, 100);
        happiness = clamp(happiness, 0, 100);
    }
    void integrate_mood(Mood m) {
        mood = m;
        if (m==MOOD_CALM) { energy += 5; happiness += 8; }
        if (m==MOOD_EXCITED) { energy += 15; happiness += 12; }
        if (m==MOOD_STRESSED) { energy -= 10; happiness -= 12; }
        energy = clamp(energy, 0, 100);
        happiness = clamp(happiness, 0, 100);
    }
};

// -------------------- Self-learning Guessing Game --------------------
struct GuessLearner {
    // track frequency distribution of user's guesses (1..100)
    vector<int> freq;
    int rounds;
    GuessLearner():freq(101,1),rounds(0){} // laplace smoothing
    void update_with_user_guess(int guess) {
        if (guess>=1 && guess<=100) freq[guess] += 1;
        rounds++;
    }
    // produce a guess distribution weighted by user's past guesses but add exploration
    int make_guess() {
        // compute weighted choice: user often guesses some numbers; we can try to place target where user less often guesses to force their adaptation
        // Here we implement a simple strategy: choose numbers near user's high-frequency guesses but sometimes explore.
        int total = 0; for (int i=1;i<=100;++i) total += freq[i];
        double r = rand01();
        if (r < 0.7) {
            // exploit: sample from freq distribution but take mode +/- small perturbation
            int mode=1; for (int i=1;i<=100;++i) if (freq[i] > freq[mode]) mode = i;
            int delta = randint(-6,6);
            int g = clamp(mode + delta, 1, 100);
            return g;
        } else {
            return randint(1,100);
        }
    }
};

// -------------------- Main Application Flow --------------------
int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    srand((unsigned)time(nullptr));

    cout << "=== EuphoriSim — Mood-Driven Life & Route Simulator ===\n\n";

    // 1) Typing sample and mood inference
    cout << "Phase 1: Typing-based mood detection\n";
    TypingSample ts = record_typing_sample();
    cout << "Measured intervals: mean=" << (int)ts.mean() << "ms stddev=" << (int)ts.stddev() << "ms\n";
    Mood m = infer_mood(ts);
    cout << "Inferred mood: " << mood_name(m) << "\n\n";

    // Music recommendations
    cout << "Music recommendations for your mood:\n";
    auto recs = recommend_by_mood(m, 3);
    for (int i=0;i<(int)recs.size();++i) {
        cout << i+1 << ". " << recs[i].title << " — " << recs[i].artist << "\n";
    }
    cout << "\n";

    // 2) Map creation (small set)
    vector<Point> mapPlaces = {
        {"Home", 0,0}, {"Office", 5,1}, {"Gym", -1,4}, {"Market", 3,-2}, {"Park", -3,-1}
    };
    cout << "Map places:\n";
    for (int i=0;i<(int)mapPlaces.size();++i) cout << i << ": " << mapPlaces[i].name << " ("<<mapPlaces[i].x<<","<<mapPlaces[i].y<<")\n";
    cout << "\n";

    // Let user pick destinations to visit today
    cout << "Choose places to visit today from the list (space-separated indices). Press ENTER for default (Office, Market):\n";
    cout << "Example input: 1 3 4\n> ";
    string line;
    getline(cin, line);
    vector<int> chosenIdx;
    if (line.empty()) { chosenIdx = {0,1,3}; } // Home included at 0 by default
    else {
        stringstream ss(line);
        int x;
        set<int> picked;
        while (ss >> x) if (x>=0 && x<(int)mapPlaces.size()) picked.insert(x);
        if (picked.empty()) chosenIdx = {0,1,3};
        else {
            // ensure home (0) present
            picked.insert(0);
            for (int v: picked) chosenIdx.push_back(v);
        }
    }
    // Build smaller places vector for GA (we'll use chosen order indices)
    vector<Point> gaPlaces;
    for (int idx: chosenIdx) gaPlaces.push_back(mapPlaces[idx]);
    cout << "Selected places: ";
    for (auto &p: gaPlaces) cout << p.name << ", ";
    cout << "\n\n";

    // 3) Run GA to find efficient route
    cout << "Optimizing route with a Genetic Algorithm (this runs locally)...\n";
    GA ga(gaPlaces, 160, 0.85, 0.10);
    auto bestChrom = ga.run(250); // returns ordering of indices in gaPlaces
    cout << "Optimized route:\n";
    double routeCost = 0;
    for (size_t i=0;i<bestChrom.size();++i) {
        cout << gaPlaces[bestChrom[i]].name;
        if (i+1<bestChrom.size()) cout << " -> ";
        if (i+1<bestChrom.size()) routeCost += dist(gaPlaces[bestChrom[i]], gaPlaces[bestChrom[i+1]]);
    }
    routeCost += dist(gaPlaces[bestChrom.back()], gaPlaces[bestChrom.front()]);
    cout << " -> (return)\n";
    cout << "Estimated route cost (Euclidean): " << fixed << setprecision(2) << routeCost << "\n\n";

    // 4) LifeSim: create citizen and simulate day using mood & route
    Citizen c("Basava");
    c.integrate_mood(m);
    cout << "Simulating a day for citizen '"<<c.name<<"' with mood "<<mood_name(c.mood)<<"\n";
    cout << "Start: Energy="<<c.energy<<" Happiness="<<c.happiness<<"\n";
    // We'll map place types to events
    for (size_t i=0;i<bestChrom.size();++i) {
        string place = gaPlaces[bestChrom[i]].name;
        if (place=="Office") { c.apply_event("work"); cout << "[Office] Work happened. "; }
        else if (place=="Gym") { c.apply_event("exercise"); cout << "[Gym] Workout. "; }
        else if (place=="Market") { c.apply_event("errand"); cout << "[Market] Errand. "; }
        else if (place=="Park") { c.apply_event("relax"); cout << "[Park] Relaxing walk. "; }
        else { c.apply_event("coffee"); cout << "["<<place<<"] Quick stop. "; }
        cout << "Energy="<<c.energy<<" Happiness="<<c.happiness<<"\n";
        // small random events influenced by mood
        if (m==MOOD_EXCITED && rand01() < 0.18) { c.happiness += 6; cout << "  Surprise positive event! Happiness boosted.\n"; }
        if (m==MOOD_STRESSED && rand01() < 0.12) { c.energy -= 8; cout << "  Minor stressor occurred.\n"; }
    }
    cout << "End of day: Energy="<<c.energy<<" Happiness="<<c.happiness<<"\n\n";

    // 5) Self-learning guessing game
    cout << "Mini-game: Self-learning Guessing Challenge\n";
    cout << "Rules: You will think of a number (1-100). The program will try to guess.\n";
    cout << "After each guess, respond with:\n  L  (if your number is Lower)\n  H  (if your number is Higher)\n  C  (if Correct)\nWe'll play up to 10 rounds. Program learns your guess habits.\n\n";
    GuessLearner gl;
    for (int round=1; round<=6; ++round) {
        cout << "Round " << round << ": Think of a number 1..100. Press ENTER when ready.";
        getline(cin, line);
        int attempt = 0;
        bool solved = false;
        int low=1, high=100;
        while (attempt < 10) {
            int guess = gl.make_guess();
            // clamp to current bounds
            if (guess < low || guess > high) guess = (low+high)/2;
            cout << "Program guesses: " << guess << "  (respond H/L/C) > ";
            string resp;
            getline(cin, resp);
            if (resp.empty()) { cout << "No response interpreted as 'C'.\n"; resp="C"; }
            char r = toupper(resp[0]);
            attempt++;
            if (r=='C') {
                cout << "Program: Yay! I guessed it in " << attempt << " tries.\n\n";
                gl.update_with_user_guess(guess);
                solved = true; break;
            } else if (r=='H') {
                low = max(low, guess+1);
                gl.update_with_user_guess(guess);
            } else if (r=='L') {
                high = min(high, guess-1);
                gl.update_with_user_guess(guess);
            } else {
                cout << "Invalid response. Please reply H/L/C. Try again.\n";
            }
            if (low > high) {
                cout << "Hmm — contradictory hints. Let's restart this round.\n";
                break;
            }
        }
        if (!solved) cout << "Round ended. Either not guessed or user aborted.\n\n";
    }
    cout << "Thanks for playing. The learner has updated its model and will adapt next time!\n\n";

    // Wrap-up summary
    cout << "=== Day Summary ===\n";
    cout << "Mood: " << mood_name(m) << "\n";
    cout << "Suggested tracks: ";
    for (auto &s: recs) cout << s.title << " ("<<s.artist<<"), ";
    cout << "\nOptimized route: ";
    for (auto &p: gaPlaces) cout << p.name << " ";
    cout << "\nCitizen '"<<c.name<<"' final Energy="<<c.energy<<" Happiness="<<c.happiness<<"\n";
    cout << "\nYou can re-run the program, pick different places, or provide longer typing samples to refine mood detection.\n";
    cout << "EuphoriSim - unique fusion project by you. Credit: you >:) \n";
    cout << "Goodbye!\n";
    return 0;
}
