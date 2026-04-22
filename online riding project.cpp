// swiftgo_custom_ds.cpp
// SwiftGo — uses custom MyStack, MyQueue, MyDeque classes (keeps vector/list for other containers).
// Features: preferred fare, admin portal, undo last assignment, auto-assign (manual scan), Display Highest Rated Driver.

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <limits>
#include <cctype>
#include <algorithm>
#include <iomanip>
//#include <map>
#include <functional>
using namespace std;

// -----------------------------
// Custom containers
// -----------------------------

// Simple LIFO stack (replacement for std::stack)
template<typename T>
class MyStack {
private:
    vector<T> data;
public:
    void push(const T &item) { data.push_back(item); }
    void pop() { if(!data.empty()) data.pop_back(); }
    T& top() { return data.back(); }
    const T& top() const { return data.back(); }
    bool empty() const { return data.empty(); }
    int size() const { return (int)data.size(); }
};

// Simple FIFO queue (replacement for std::queue) — implemented but not heavily used
template<typename T>
class MyQueue {
private:
    vector<T> data;
    size_t frontIndex = 0;
public:
    void push(const T &item) { data.push_back(item); }
    void pop() { if(!empty()) ++frontIndex; }
    T& front() { return data[frontIndex]; }
    const T& front() const { return data[frontIndex]; }
    bool empty() const { return frontIndex >= data.size(); }
    int size() const { return (int)(data.size() - frontIndex); }
};

// Simple deque with iterator support (replacement for std::deque)
template<typename T>
class MyDeque {
private:
    vector<T> data;
public:
    void push_back(const T &item) { data.push_back(item); }
    void push_front(const T &item) { data.insert(data.begin(), item); }
    void pop_front() { if(!data.empty()) data.erase(data.begin()); }
    T& front() { return data.front(); }
    const T& front() const { return data.front(); }
    T& back() { return data.back(); }
    const T& back() const { return data.back(); }
    bool empty() const { return data.empty(); }
    int size() const { return (int)data.size(); }
    auto begin() { return data.begin(); }
    auto end() { return data.end(); }
    auto begin() const { return data.begin(); }
    auto end() const { return data.end(); }
};

// -----------------------------
// ANSI color codes
// -----------------------------
#define RESET   "\033[0m"
#define CYAN    "\033[0m\033[36m"
#define GREY    "\033[0m\033[90m"

// -----------------------------
// Utility functions
// -----------------------------
string formatID(int id) {
    string s = to_string(id);
    while(s.length()<3) s = "0" + s;
    return s;
}

bool isValidName(const string &name) {
    int letters = 0, spaces = 0;
    for(char c: name){
        if(isalpha((unsigned char)c)) letters++;
        else if(c==' ') spaces++;
        else return false;
    }
    return letters>0 && spaces<=letters;
}

bool isValidDoubleInput(double &num) {
    if(!(cin >> num)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); return false; }
    return true;
}

bool isValidIntInput(int &num) {
    if(!(cin >> num)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); return false; }
    return true;
}

// -----------------------------
// Data models
// -----------------------------
struct Driver {
    int id;
    string name;
    double rating = 0.0; // average rating
    double distance;
    bool available;
    int totalRides = 0; // for calculating average rating
};

struct User {
    int id;
    string name;
};

// RideRequest includes preferredFare
struct RideRequest {
    int requestId;
    int userId;
    string pickup;
    string dropoff;
    double distance;
    double preferredFare;
};

// AssignedRide includes fare (initially preferredFare)
struct AssignedRide {
    int requestId;
    int userId;
    int driverId;
    string pickup;
    string dropoff;
    double distance;
    double fare;       // fare agreed / stored (initially preferred fare)
    double rating = -1; // -1 = not rated yet
};

// -----------------------------
// Driver Manager
// -----------------------------
class DriverManager {
private:
    list<Driver> drivers;
    int nextDriverId = 1;
public:
    int addDriver(const string &name, double distance) {
        drivers.push_back({nextDriverId++, name, 0.0, distance, true, 0});
        return drivers.back().id;
    }

    Driver* findDriver(int id) {
        for (auto &d : drivers) if (d.id == id) return &d;
        return nullptr;
    }

    // Return available driver ids as a vector
    vector<int> getAvailableDriverIds() {
        vector<int> out;
        for (auto &d : drivers) if (d.available) out.push_back(d.id);
        return out;
    }

    void setAvailability(int id, bool avail) {
        Driver* d = findDriver(id);
        if(d) d->available = avail;
    }

    int totalDrivers() { return (int)drivers.size(); }

    int activeDrivers() {
        int cnt=0;
        for (auto &d : drivers) if (d.available) ++cnt;
        return cnt;
    }

    void updateRating(int id, double newRating) {
        Driver* d = findDriver(id);
        if(!d) return;
        d->rating = ((d->rating * d->totalRides) + newRating) / (d->totalRides + 1);
        d->totalRides++;
    }

    void decrementRideCountAndAdjustRating(int id, double ratingRemoved) {
        Driver* d = findDriver(id);
        if(!d || d->totalRides<=0) return;
        double totalScore = d->rating * d->totalRides;
        totalScore -= ratingRemoved;
        d->totalRides--;
        if(d->totalRides>0) d->rating = totalScore / d->totalRides;
        else { d->rating = 0.0; d->totalRides = 0; }
    }

    void displayAll() {
        cout << CYAN << "\n========== DRIVERS ==========\n" << RESET;
        if(drivers.empty()) { cout << GREY << "No drivers added yet.\n" << RESET; return; }
        cout << GREY << left << setw(6) << "ID"
             << setw(20) << "Name"
             << setw(8) << "Rating"
             << setw(10) << "DistPref"
             << setw(12) << "Status" << RESET << '\n';
        cout << string(56,'-') << '\n';
        for (auto &drv : drivers) {
            cout << GREY << left << setw(6) << formatID(drv.id)
                 << setw(20) << drv.name
                 << setw(8) << fixed << setprecision(2) << drv.rating
                 << setw(10) << drv.distance
                 << setw(12) << (drv.available?"Available":"Busy") << RESET << '\n';
        }
    }

    void displayDriverShort(int id) {
        Driver* d = findDriver(id);
        if(!d) { cout << GREY << "Driver not found\n" << RESET; return; }
        cout << GREY << "Driver: " << d->name << " (ID:" << formatID(d->id) << ") "
             << (d->available? "Available":"Busy") << " Rating:" << d->rating << " PrefDist:" << d->distance << RESET << '\n';
    }

    // Display top 1 driver by rating (Option A = across ALL drivers)
    void displayTopDriver() {
        if(drivers.empty()){
            cout << GREY << "Top driver: none (no drivers registered)\n" << RESET;
            return;
        }

        // Check if ALL drivers have rating 0 ? no meaningful top driver
        bool allZero = true;
        for (const auto &d : drivers) {
            if (d.rating > 0) { allZero = false; break; }
        }
        if (allZero) {
            cout << GREY << "Top driver: none (no driver has been rated yet)\n" << RESET;
            return;
        }

        // Manual scan to find top driver (highest rating, tie => smaller distance)
        const Driver* best = nullptr;
        for (const auto &d : drivers) {
            if (!best) { best = &d; continue; }
            if (d.rating > best->rating) best = &d;
            else if (d.rating == best->rating && d.distance < best->distance) best = &d;
        }

        if (!best || best->rating <= 0) {
            cout << GREY << "Top driver: none (no driver has been rated yet)\n" << RESET;
            return;
        }

        cout << GREY << "Top Driver: " << best->name
             << " (ID:" << formatID(best->id) << ") Rating: " << fixed << setprecision(2) << best->rating
             << " PrefDist: " << best->distance << RESET << '\n';
    }
};

// -----------------------------
// User Manager
// -----------------------------
class UserManager {
private:
    list<User> users;
    int nextId = 1;
public:
    int addUser(const string &name) {
        users.push_back({nextId++, name});
        return users.back().id;
    }

    User* findUser(int id) {
        for (auto &u : users) if (u.id == id) return &u;
        return nullptr;
    }

    int totalUsers() { return (int)users.size(); }

    void displayAll() {
        cout << CYAN << "\n========== USERS ==========\n" << RESET;
        if(users.empty()) { cout << GREY << "No users yet.\n" << RESET; return; }
        cout << GREY << left << setw(6) << "ID" << setw(20) << "Name" << RESET << '\n';
        cout << string(26,'-') << '\n';
        for (auto &u : users) {
            cout << GREY << left << setw(6) << formatID(u.id) << setw(20) << u.name << RESET << '\n';
        }
    }
};

// -----------------------------
// Ride Manager
// -----------------------------
class RideManager {
private:
    MyDeque<RideRequest> pending;
    list<AssignedRide> completed;
    MyStack<AssignedRide> undoStack; // replaced vector/stack with MyStack
    int nextReqId = 1;

public:
    int addRequest(int userId, const string &pickup, const string &dropoff, double distance, double preferredFare) {
        pending.push_back({nextReqId++, userId, pickup, dropoff, distance, preferredFare});
        return pending.back().requestId;
    }

    bool popMaxDistanceRequest(RideRequest &out) {
        if(pending.empty()) return false;
        auto itMax = pending.begin();
        double maxD = itMax->distance;
        for(auto it = pending.begin(); it != pending.end(); ++it){
            if(it->distance > maxD){ maxD = it->distance; itMax = it; }
        }
        out = *itMax;
        // erase element at position itMax by computing index
        int idx = 0;
        for(auto it = pending.begin(); it != itMax; ++it) ++idx;
        // remove element at idx
        // since MyDeque doesn't have erase by index, we'll reconstruct
        MyDeque<RideRequest> tmp;
        int i = 0;
        for(auto it = pending.begin(); it != pending.end(); ++it){
            if(i++ == idx) continue;
            tmp.push_back(*it);
        }
        pending = std::move(tmp);
        return true;
    }

    bool popRequestById(int requestId, RideRequest &outRequest) {
        int idx = 0;
        bool found = false;
        for(auto it = pending.begin(); it != pending.end(); ++it, ++idx) {
            if(it->requestId == requestId) {
                outRequest = *it;
                found = true;
                break;
            }
        }
        if(!found) return false;
        // rebuild deque without that index
        MyDeque<RideRequest> tmp;
        int i = 0;
        for(auto it = pending.begin(); it != pending.end(); ++it){
            if(i++ == idx) continue;
            tmp.push_back(*it);
        }
        pending = std::move(tmp);
        return true;
    }

    void pushFront(const RideRequest &r) {
        pending.push_front(r);
    }

    int pendingCount() { return pending.size(); }

    void addHistory(const AssignedRide &a) {
        completed.push_back(a);
        undoStack.push(a); // push into custom stack
    }

    void markRideCompleted(int driverId, int requestId, DriverManager &drivers, UserManager &users) {
        AssignedRide *target = nullptr;
        for(auto &r : completed){
            if(r.driverId==driverId && r.requestId==requestId && r.rating < 0){
                target = &r; break;
            }
        }
        if(!target){
            cout << GREY << "Error: Ride not found or already completed.\n" << RESET;
            return;
        }
        drivers.setAvailability(driverId,true);
        cout << GREY << "Ride " << formatID(requestId) << " marked completed. You are now available.\n" << RESET;

        // Prompt user rating
        User *user = users.findUser(target->userId);
        if(!user) return;
        double r;
        while(true){
            cout << CYAN << "Hi " << user->name << "! Please rate your ride " << formatID(requestId) << " (0-5): " << RESET;
            if(!(cin >> r) || r<0 || r>5){
                cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n');
                cout << GREY << "Invalid rating. Try again.\n" << RESET;
                continue;
            }
            break;
        }
        target->rating = r;
        drivers.updateRating(driverId, r);
        cout << GREY << "Thank you! You rated this ride " << r << "\n" << RESET;
    }

    void displayPending() {
        cout << CYAN << "\n====== PENDING REQUESTS ======\n" << RESET;
        if(pending.empty()){ cout << GREY << "No pending requests.\n" << RESET; return; }
        cout << GREY << left << setw(6) << "ReqID" << setw(6) << "UID"
             << setw(12) << "Pickup" << setw(12) << "Dropoff" << setw(10) << "Distance" << setw(10) << "Fare" << RESET << '\n';
        cout << string(66,'-') << '\n';
        for (auto &pr : pending) {
            cout << GREY << left << setw(6) << formatID(pr.requestId)
                 << setw(6) << formatID(pr.userId)
                 << setw(12) << pr.pickup
                 << setw(12) << pr.dropoff
                 << setw(10) << pr.distance
                 << setw(10) << fixed << setprecision(2) << pr.preferredFare << RESET << '\n';
        }
    }

    void displayHistoryForUser(int userId) {
        cout << CYAN << "\n====== YOUR RIDE HISTORY ======\n" << RESET;
        bool any=false;
        cout << GREY << left << setw(6) << "ReqID" << setw(6) << "DID" << setw(12) << "Pickup" << setw(12) << "Dropoff" << setw(10) << "Distance" << setw(8) << "Fare" << setw(6) << "Rating" << RESET << '\n';
        cout << string(72,'-') << '\n';
        for (auto &h : completed) {
            if(h.userId == userId) {
                any=true;
                cout << GREY << left << setw(6) << formatID(h.requestId)
                     << setw(6) << formatID(h.driverId)
                     << setw(12) << h.pickup
                     << setw(12) << h.dropoff
                     << setw(10) << h.distance
                     << setw(8) << fixed << setprecision(2) << h.fare
                     << setw(6) << (h.rating<0 ? "-" : to_string(h.rating)) << RESET << '\n';
            }
        }
        if(!any) cout << GREY << "No rides yet.\n" << RESET;
    }

    void displayAssignedToDriver(int driverId) {
        cout << CYAN << "\n====== YOUR ASSIGNED RIDES ======\n" << RESET;
        bool any=false;
        cout << GREY << left << setw(6) << "ReqID" << setw(6) << "UID" << setw(12) << "Pickup" << setw(12) << "Dropoff" << setw(10) << "Distance" << setw(8) << "Fare" << setw(6) << "Rating" << RESET << '\n';
        cout << string(72,'-') << '\n';
        for (auto &h : completed) {
            if(h.driverId == driverId && h.rating<0) {
                any=true;
                cout << GREY << left << setw(6) << formatID(h.requestId)
                     << setw(6) << formatID(h.userId)
                     << setw(12) << h.pickup
                     << setw(12) << h.dropoff
                     << setw(10) << h.distance
                     << setw(8) << fixed << setprecision(2) << h.fare
                     << setw(6) << "-" << RESET << '\n';
            }
        }
        if(!any) cout << GREY << "No assigned rides yet.\n" << RESET;
    }

    // Display full completed history (admin)
    void displayAllHistory() {
        cout << CYAN << "\n====== ALL ASSIGNED/COMPLETED RIDES ======\n" << RESET;
        if(completed.empty()) { cout << GREY << "No rides in history.\n" << RESET; return; }
        cout << GREY << left << setw(6) << "ReqID" << setw(6) << "UID" << setw(6) << "DID"
             << setw(12) << "Pickup" << setw(12) << "Dropoff" << setw(10) << "Distance" << setw(8) << "Fare" << setw(8) << "Rating" << RESET << '\n';
        cout << string(86,'-') << '\n';
        for (auto &h : completed) {
            cout << GREY << left << setw(6) << formatID(h.requestId)
                 << setw(6) << formatID(h.userId)
                 << setw(6) << formatID(h.driverId)
                 << setw(12) << h.pickup
                 << setw(12) << h.dropoff
                 << setw(10) << h.distance
                 << setw(8) << fixed << setprecision(2) << h.fare
                 << setw(8) << (h.rating<0 ? "-" : to_string(h.rating)) << RESET << '\n';
        }
    }

    // Undo last assignment (only if not rated yet). Returns true if undone and pushes request back to pending front.
    bool undoLastAssignment(DriverManager &drivers) {
        if(undoStack.empty()){
            cout << GREY << "No assignments to undo.\n" << RESET;
            return false;
        }
        AssignedRide last = undoStack.top();

        // If ride was rated, do not undo (to avoid complications reversing ratings)
        if(last.rating >= 0) {
            cout << GREY << "Cannot undo: last assignment already rated.\n" << RESET;
            return false;
        }

        // Find and remove from completed list the matching assigned ride (searching from end to find most recent)
        bool removed = false;
        for(auto it = completed.end(); it != completed.begin(); ){
            --it;
            if(it->requestId == last.requestId && it->driverId == last.driverId) {
                completed.erase(it);
                removed = true;
                break;
            }
        }
        if(!removed){
            for(auto it = completed.begin(); it != completed.end(); ++it){
                if(it->requestId == last.requestId && it->driverId == last.driverId) {
                    completed.erase(it);
                    removed = true;
                    break;
                }
            }
        }

        // pop undo stack
        undoStack.pop();

        // Restore driver availability
        drivers.setAvailability(last.driverId, true);

        // Recreate ride request (with preferredFare restored) and push to front of pending
        RideRequest restored{ last.requestId, last.userId, last.pickup, last.dropoff, last.distance, last.fare };
        pushFront(restored);

        cout << GREY << "Undid assignment: Request " << formatID(last.requestId) << " restored to pending and Driver " << formatID(last.driverId) << " is now available.\n" << RESET;
        return true;
    }
};

// -----------------------------
// Ride System
// -----------------------------
class RideSystem {
private:
    DriverManager drivers;
    UserManager users;
    RideManager rides;

public:
    DriverManager& getDrivers() { return drivers; }
    UserManager& getUsers() { return users; }
    RideManager& getRides() { return rides; }

    int addDriver(const string &name, double distance){
        return drivers.addDriver(name, distance);
    }

    int addUser(const string &name){
        return users.addUser(name);
    }

    // bookRide now takes preferredFare
    int bookRide(int userId, const string &pickup, const string &dropoff, double distance, double preferredFare){
        if(!users.findUser(userId)) throw runtime_error("User not found");
        return rides.addRequest(userId, pickup, dropoff, distance, preferredFare);
    }

    void assignNextAuto(){
        RideRequest req;
        if(!rides.popMaxDistanceRequest(req)){ cout << GREY << "No requests to assign.\n" << RESET; return; }

        vector<int> availIds = drivers.getAvailableDriverIds();
        if(availIds.empty()){
            rides.pushFront(req);
            cout << GREY << "No available drivers. Request remains pending.\n" << RESET;
            return;
        }

        // Manual scan to find the driver with maximum score
        int bestId = -1;
        double bestScore = numeric_limits<double>::lowest();
        double bestRating = -1;
        double bestDistance = numeric_limits<double>::max();

        for (int id : availIds) {
            Driver* d = drivers.findDriver(id);
            if(!d) continue;
            double score = d->distance + (5.0 - d->rating) * 2.0; // same formula as before
            if (score > bestScore) {
                bestScore = score;
                bestId = d->id;
                bestRating = d->rating;
                bestDistance = d->distance;
            } else if (score == bestScore) {
                // tie-breaker: prefer higher rating, then smaller distance
                if (d->rating > bestRating || (d->rating == bestRating && d->distance < bestDistance)) {
                    bestId = d->id;
                    bestRating = d->rating;
                    bestDistance = d->distance;
                }
            }
        }

        if(bestId == -1){
            rides.pushFront(req);
            cout << GREY << "No available drivers after scoring.\n" << RESET;
            return;
        }

        Driver* bestDriver = drivers.findDriver(bestId);
        if(!bestDriver){
            rides.pushFront(req);
            cout << GREY << "Driver not found.\n" << RESET;
            return;
        }

        // Assign using preferredFare as initial fare
        AssignedRide a{req.requestId, req.userId, bestDriver->id, req.pickup, req.dropoff, req.distance, req.preferredFare};
        drivers.setAvailability(bestDriver->id, false);
        rides.addHistory(a);

        cout << GREY << "Auto-assigned Driver " << bestDriver->name << " (ID:" << formatID(bestDriver->id)
             << ") to Request " << formatID(req.requestId) << " (Distance: " << req.distance << ", Fare: " << fixed << setprecision(2) << req.preferredFare << ")" << RESET << '\n';
    }

    bool driverAcceptRequest(int driverId, int requestId) {
        Driver* d = drivers.findDriver(driverId);
        if(!d) { cout << GREY << "Driver not found\n" << RESET; return false; }
        if(!d->available) { cout << GREY << "You are currently busy. Complete your assigned rides first.\n" << RESET; return false; }

        RideRequest req;
        if(!rides.popRequestById(requestId, req)) {
            cout << GREY << "Request not found or already taken.\n" << RESET;
            return false;
        }

        AssignedRide a{req.requestId, req.userId, driverId, req.pickup, req.dropoff, req.distance, req.preferredFare};
        drivers.setAvailability(driverId, false);
        rides.addHistory(a);
        cout << GREY << "You accepted Request " << formatID(req.requestId) << " (Pickup: " << req.pickup << ", Distance: " << req.distance << ", Fare: " << fixed << setprecision(2) << req.preferredFare << ")\n" << RESET;
        return true;
    }

    void driverCompleteRide(int driverId, int requestId) {
        rides.markRideCompleted(driverId, requestId, drivers, users);
    }

    // Expose undo to admin
    void undoLastAssignment() {
        rides.undoLastAssignment(drivers);
    }
};

// -----------------------------
// Dashboard & Menu
// -----------------------------
void showSystemDashboard(RideSystem &sys){
    cout << CYAN << "\n===== SWIFTGO DASHBOARD =====\n" << RESET;
    cout << GREY << "Total Users      : " << sys.getUsers().totalUsers() << '\n';
    cout << "Total Drivers    : " << sys.getDrivers().totalDrivers() << '\n';
    cout << "Active Drivers   : " << sys.getDrivers().activeDrivers() << '\n';
    cout << "Pending Requests : " << sys.getRides().pendingCount() << RESET << '\n';
}

// -----------------------------
// User Portal
// -----------------------------
void userPortal(RideSystem &sys) {
    while(true){
        cout << CYAN << "\n--- USER PORTAL ---\n" << RESET;
        cout << GREY << "1. Create Account\n2. Login with User ID\n0. Back\n" << RESET;
        cout << "Choose: ";
        int opt; if(!isValidIntInput(opt)){ cout << GREY << "Invalid input\n" << RESET; continue; }
        if(opt==0) break;
        if(opt==1){
            cout << "Enter name: "; string name; cin>>ws; getline(cin,name);
            if(!isValidName(name)){ cout << GREY << "Invalid name!\n" << RESET; continue; }
            int id = sys.addUser(name);
            cout << GREY << "User created. ID: " << formatID(id) << RESET << '\n';
        } else if(opt==2){
            cout << "Enter User ID: "; int uid; if(!isValidIntInput(uid)){ cout << GREY << "Invalid ID\n" << RESET; continue; }
            User* u = sys.getUsers().findUser(uid);
            if(!u){ cout << GREY << "User not found\n" << RESET; continue; }
            while(true){
                cout << CYAN << "\n--- USER MENU ---\n" << RESET;
                cout << GREY << "1. Book Ride\n2. View My Ride History\n3. View Pending Requests\n0. Logout\n" << RESET;
                cout << "Choose: ";
                int uopt; if(!isValidIntInput(uopt)){ cout << GREY << "Invalid input\n" << RESET; continue; }
                if(uopt==0) break;
                if(uopt==1){
                    string pu, dof; double dist; double prefFare;
                    cout << "Pickup: "; cin>>ws; getline(cin, pu);
                    cout << "Dropoff: "; cin>>ws; getline(cin, dof);
                    cout << "Distance: "; if(!isValidDoubleInput(dist)) { cout << GREY << "Invalid distance\n" << RESET; continue; }
                    cout << "Preferred Fare: "; if(!isValidDoubleInput(prefFare)) { cout << GREY << "Invalid fare\n" << RESET; continue; }
                    int rid = sys.bookRide(uid, pu, dof, dist, prefFare);
                    cout << GREY << "Ride Request Created. ReqID: " << formatID(rid) << " (Fare: " << fixed << setprecision(2) << prefFare << ")" << RESET << '\n';
                } else if(uopt==2){
                    sys.getRides().displayHistoryForUser(uid);
                } else if(uopt==3){
                    sys.getRides().displayPending();
                }
            }
        }
    }
}

// -----------------------------
// Driver Portal
// -----------------------------
void driverPortal(RideSystem &sys){
    while(true){
        cout << CYAN << "\n--- DRIVER PORTAL ---\n" << RESET;
        cout << GREY << "1. Register Driver\n2. Accept Ride Request\n3. Complete Ride\n4. View My Assigned Rides\n0. Back\n" << RESET;
        cout << "Choose: "; int opt; if(!isValidIntInput(opt)){ cout << GREY << "Invalid input\n" << RESET; continue; }
        if(opt==0) break;
        if(opt==1){
            cout << "Name: "; string n; cin>>ws; getline(cin,n);
            if(!isValidName(n)){ cout << GREY << "Invalid name\n" << RESET; continue; }
            cout << "Distance preference: "; double d; if(!isValidDoubleInput(d)){ cout << GREY << "Invalid input\n" << RESET; continue; }
            int id = sys.addDriver(n,d);
            cout << GREY << "Driver registered. ID: " << formatID(id) << RESET << '\n';
        } else if(opt==2){
            sys.getRides().displayPending();
            cout << "Enter Request ID to accept: "; int rid; if(!isValidIntInput(rid)){ cout << GREY << "Invalid ID\n" << RESET; continue; }
            cout << "Enter your Driver ID: "; int did; if(!isValidIntInput(did)){ cout << GREY << "Invalid ID\n" << RESET; continue; }
            sys.driverAcceptRequest(did,rid);
        } else if(opt==3){
            cout << "Enter Request ID to complete: "; int rid; if(!isValidIntInput(rid)){ cout << GREY << "Invalid ID\n" << RESET; continue; }
            cout << "Enter your Driver ID: "; int did; if(!isValidIntInput(did)){ cout << GREY << "Invalid ID\n" << RESET; continue; }
            sys.driverCompleteRide(did,rid);
        } else if(opt==4){
            cout << "Enter your Driver ID: "; int did; if(!isValidIntInput(did)){ cout << GREY << "Invalid ID\n" << RESET; continue; }
            sys.getRides().displayAssignedToDriver(did);
        }
    }
}

// -----------------------------
// Admin Portal
// -----------------------------
void adminPortal(RideSystem &sys){
    // For simplicity, no password. Add one if needed.
    while(true){
        cout << CYAN << "\n--- ADMIN PORTAL ---\n" << RESET;
        cout << GREY << "1. View All Drivers\n2. View All Users\n3. View All Ride History\n4. Undo Last Assignment (only if not rated)\n0. Back\n" << RESET;
        cout << "Choose: "; int opt; if(!isValidIntInput(opt)){ cout << GREY << "Invalid input\n" << RESET; continue; }
        if(opt==0) break;
        if(opt==1) sys.getDrivers().displayAll();
        else if(opt==2) sys.getUsers().displayAll();
        else if(opt==3) sys.getRides().displayAllHistory();
        else if(opt==4) sys.undoLastAssignment();
    }
}

// -----------------------------
// Main
// -----------------------------
int main(){
    

    RideSystem sys;
    while(true){
        showSystemDashboard(sys);
        cout << CYAN << "\n--- MAIN MENU ---\n" << RESET;
        cout << GREY 
             << "1. User Portal\n"
             << "2. Driver Portal\n"
             << "3. Auto-assign next pending ride\n"
             << "4. Admin Portal\n"
             << "5. Display Highest Rated Driver\n"
             << "0. Exit\n" << RESET;
        cout << "Choose: "; int opt; if(!isValidIntInput(opt)){ cout << GREY << "Invalid input\n" << RESET; continue; }

        if(opt==0) break;
        else if(opt==1) userPortal(sys);
        else if(opt==2) driverPortal(sys);
        else if(opt==3) sys.assignNextAuto();
        else if(opt==4) adminPortal(sys);
        else if(opt==5) sys.getDrivers().displayTopDriver();
    }
    cout << GREY << "Exiting SwiftGo. Goodbye!\n" << RESET;
    return 0;
}
