#include <bits/stdc++.h>
using namespace std;

// Simple structure for typing time
struct Typing {
    vector<double> gaps;
    double mean() {
        if (gaps.empty()) return 0;
        double s = 0;
        for (auto x : gaps) s += x;
        return s / gaps.size();
    }
};

// Detect mood using average typing delay
string findMood(double avg) {
    if (avg < 120) return "Excited 😄";
    else if (avg < 160) return "Normal 🙂";
    else if (avg < 250) return "Calm 😌";
    else return "Stressed 😓";
}

// Suggest songs based on mood
void suggestSongs(string mood) {
    cout << "\n--- Music Suggestions ---\n";
    if (mood.find("Excited") != string::npos) {
        cout << "1. Neon Pulse - Jetstream\n2. Uplift - StellarVox\n";
    } else if (mood.find("Calm") != string::npos) {
        cout << "1. Coffee & Rain - Slowfold\n2. Easy Sunday - Paper Boat\n";
    } else if (mood.find("Normal") != string::npos) {
        cout << "1. Focus Mode - Binary Hearts\n2. Walking Home - Alleyways\n";
    } else {
        cout << "1. Quiet Corners - MellowMuse\n2. Sunrise Drive - Nova Lane\n";
    }
    cout << "--------------------------\n";
}

// Route optimization (simple distance-based)
struct Point {
    string name;
    double x, y;
};

double dist(Point a, Point b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

// Life simulation
struct Citizen {
    string name;
    int energy;
    int happy;
    string mood;
};

void simulateDay(Citizen &c, vector<Point> route) {
    cout << "\n--- Daily Simulation ---\n";
    cout << "Citizen: " << c.name << " | Mood: " << c.mood << endl;
    for (auto p : route) {
        cout << "Visiting: " << p.name << " ... ";
        if (p.name == "Office") {
            c.energy -= 20; c.happy += 5;
            cout << "Worked hard!\n";
        } else if (p.name == "Gym") {
            c.energy -= 15; c.happy += 10;
            cout << "Good workout!\n";
        } else if (p.name == "Park") {
            c.energy += 10; c.happy += 8;
            cout << "Relaxed in park!\n";
        } else {
            c.energy -= 10; c.happy += 2;
            cout << "Small errand done!\n";
        }
        if (c.energy < 0) c.energy = 0;
        if (c.happy > 100) c.happy = 100;
    }
    cout << "End of day -> Energy: " << c.energy << ", Happiness: " << c.happy << endl;
    cout << "-------------------------\n";
}

int main() {
    srand(time(0));
    cout << "====== EuphoriSim - Mood Based Day Simulator ======\n";
    
    // Mood detection
    cout << "\nStep 1: Type a short sentence and press Enter:\n";
    cin.ignore();
    string text;
    getline(cin, text);
    
    Typing t;
    // fake random typing intervals
    for (int i = 0; i < (int)text.size(); i++)
        t.gaps.push_back(80 + rand() % 200);
    
    double avg = t.mean();
    string mood = findMood(avg);
    cout << "\nYour average typing delay: " << (int)avg << " ms\n";
    cout << "Detected Mood: " << mood << endl;
    
    suggestSongs(mood);
    
    // Step 2: Map route (simple)
    vector<Point> places = {
        {"Home", 0, 0}, {"Office", 4, 2}, {"Gym", -2, 3}, {"Park", 1, -4}, {"Market", 3, -2}
    };
    
    cout << "\nAvailable Places:\n";
    for (int i = 0; i < (int)places.size(); i++)
        cout << i << ". " << places[i].name << endl;
    
    cout << "Enter 3 place numbers to visit today (like 1 3 4): ";
    vector<Point> route;
    int a, b, c;
    cin >> a >> b >> c;
    route.push_back(places[0]); // always start from home
    route.push_back(places[a]);
    route.push_back(places[b]);
    route.push_back(places[c]);
    route.push_back(places[0]); // return home
    
    double total = 0;
    for (int i = 0; i < (int)route.size() - 1; i++)
        total += dist(route[i], route[i + 1]);
    cout << "Estimated travel distance: " << fixed << setprecision(2) << total << " units\n";
    
    // Step 3: Simulate life
    Citizen me;
    me.name = "Basava";
    me.energy = 80;
    me.happy = 70;
    me.mood = mood;
    
    simulateDay(me, route);
    
    // Step 4: Small guessing game
    cout << "\nLet's play a quick guessing game!\n";
    int secret = rand() % 50 + 1, guess;
    for (int tries = 1; tries <= 5; tries++) {
        cout << "Try " << tries << ": Guess a number (1-50): ";
        cin >> guess;
        if (guess == secret) {
            cout << "🎉 Correct! You guessed it!\n";
            break;
        } else if (guess < secret)
            cout << "Higher!\n";
        else
            cout << "Lower!\n";
    }
    
    cout << "\n===== Summary =====\n";
    cout << "Mood: " << mood << endl;
    cout << "Total Travel Distance: " << total << endl;
    cout << "Final Energy: " << me.energy << endl;
    cout << "Final Happiness: " << me.happy << endl;
    cout << "===================\n";
    
    cout << "Thanks for using EuphoriSim! 😊\n";
    return 0;
}
