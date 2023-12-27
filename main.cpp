#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <queue>
#include <set>
#include <float.h>
#include <regex>
#include <cmath>
#include <algorithm>

using namespace std;

struct CampData {
    double lat;
    double lon;
    string camp;
    string state;
    string name;
    string type;
    string phone;
    string url;
    string town;
    int sites = 1;
    int camp_id;
    // made all of these variables boolean, so we can simplify the search and printing for these variables.
    bool hookups;
    bool toilets;
    bool water;
    bool url_confirmed;
    bool reservations;
    bool dump;
    bool showers;
    bool pets;
    bool cheap;
    bool approved = true;
    //most important one^, decides if campsite gets added to the map. Gets switched off when a requirement isn't met

    // this will store the nearby neighbors for every campsite
    vector<CampData*> neighbors;
    CampData* parent = nullptr;
    double f;
};
double calculateDistance(CampData &origin, CampData &dest) {
    double dist;
    constexpr double Pi = 3.14159265358979323846;
    double lat1 = origin.lat * (Pi / 180.0);
    double lon1 = origin.lon * (Pi / 180.0);
    double lat2 = dest.lat * (Pi / 180.0);
    double lon2 = dest.lon * (Pi / 180.0);
    double deltaLat = lat2 - lat1;
    double deltaLon = lon2 - lon1;
    double havDeltaLat = sin(deltaLat / 2) * sin(deltaLat / 2);
    double havDeltaLon = sin(deltaLon / 2) * sin(deltaLon / 2);
    double a = havDeltaLat + cos(lat1) * cos(lat2) * havDeltaLon;
    // Earth's mean radius in miles
    constexpr double EarthRadiusMiles = 3959.0;
    dist = EarthRadiusMiles * 2 * atan2(sqrt(a), sqrt(1 - a));
    return dist;
}

// this function takes in the map and the distance threshold and fills the neighbors vector with neighbors within the
// distance threshold
void createNeighbors(map<string, CampData> &camps, double distanceThreshold) {
    //loop through every node and calculate if its long and lat is within neighbor distance of the other node
    auto it = camps.begin();
    while (next(it) != camps.end()) {
        for (auto jt = next(it); jt != camps.end(); jt++) {
            // these^ loop should optimize this function to not loop through every campsite multiple times
            double distance = calculateDistance(it->second, jt->second);
            if (distance < distanceThreshold) {
                // add both as neighbors only once to each other since every neighbor also has the node as a neighbor
                it->second.neighbors.push_back(&jt->second);
                jt->second.neighbors.push_back(&it->second);
            }
        }
        if (it->second.neighbors.size() == 0) {
            auto ik = it;
            it++;
            camps.erase(ik);
        }
        else {
            it++;
        }
    }
}

void createNeighbors(map<string, CampData> &camps) {
    //loop through every node and calculate if its long and lat is within neighbor distance of the other node
    auto it = camps.begin();
    while (next(it) != camps.end()) {
        for (auto jt = next(it); jt != camps.end(); jt++) {
            // this^ loop should optimize this function to not loop through every campsite multiple times
                // add both as neighbors only once to each other since every neighbor also has the node as a neighbor
            it->second.neighbors.push_back(&jt->second);
            jt->second.neighbors.push_back(&it->second);
        }
        it++;
    }
}

struct CompareDist {
    bool operator()(const pair<double, CampData*>& a, const pair<double, CampData*>& b) {
        return a.first > b.first;
    }
};

// Modified Dijkstra's //
vector<CampData> DijkstrasAlgo(map<string, CampData> &camps, const string& oriCity, const string& destCity) {
    vector<CampData> result;
    priority_queue<pair<double, CampData*>, vector<pair<double, CampData*>>, CompareDist> candidates;
    set<CampData*> visited;

    auto itStart = camps.find(oriCity);

    if (itStart == camps.end()) {
        cerr << "Error: Origin city missing from map." << endl;
        return result;
    }

    auto itEnd = camps.find(destCity);

    if (itEnd == camps.end()) {
        cerr << "Error: Destination city missing from map." << endl;
        return result;
    }

    CampData* start = &itStart->second;
    CampData* end = &itEnd->second;

    candidates.push({0.0, start});

    while (!candidates.empty()) {
        CampData* temp = candidates.top().second;
        double currentDistance = candidates.top().first;
        candidates.pop();

        if (temp == end) {
            result.push_back(*temp);
            break;
        }

        if (visited.find(temp) == visited.end()) {
            visited.insert(temp);
            result.push_back(*temp);

            // Loop through neighbors
            for (CampData* neighbor : temp->neighbors) {
                if (visited.find(neighbor) == visited.end()) {
                    double potentialDistance = currentDistance + calculateDistance(*temp, *neighbor);
                    candidates.push({potentialDistance, neighbor});
                }
            }
        }
    }

    return result;
}


struct CompareAStar {
    bool operator()(const pair<double, CampData*>& a, const pair<double, CampData*>& b) {
        return (a.first + a.second->sites) > (b.first + b.second->sites);
    }
};

vector<CampData> AStar(map<string, CampData> &camps, const string& oriCity, const string& destCity) {
    vector<CampData> result;
    priority_queue<pair<double, CampData*>, vector<pair<double, CampData*>>, CompareAStar> openList;
    set<CampData*> closedList;

    // Initialize the open list
    //2.  Initialize the closed list

    auto Iterator1 = camps.find(oriCity);
    auto Iterator2 = camps.find(destCity);

    CampData* start = &Iterator1->second;
    CampData* end = &Iterator2->second;
    double gScore, hScore, fScore;

    if (Iterator1 == camps.end() || Iterator2 == camps.end()) {
        cerr << "Error: Origin or destination city missing from map." << endl;
        return result;
    }

    // Comments based on A* implementation from GeeksForGeeks
    // https://www.geeksforgeeks.org/a-search-algorithm/
    // Step 2
    openList.push({0.0, start});

    // Step 3
    // while the open list is not empty

    while (!openList.empty()) {
        // Step   a) find the node with the least f on
        //       the open list, call it "q"
        CampData* current = openList.top().second;

        // Step 3b
        //  b) pop q off the open list
        openList.pop();

        //  c) generate q's successors and set their
        //       parents to q
        for (CampData* successor : current->neighbors) {
            if (closedList.find(successor) != closedList.end()) {
                continue;
            }

            //   d) for each successor
            //  i) if successor is the goal, stop search
            if (successor->camp == end->camp) {
                // Stop search
                // Reconstruct the path and return result
                result.push_back(*successor);
                while (current != nullptr) {

                    result.push_back(*current);
                    current = current->parent;
                }
                reverse(result.begin(), result.end());
                return result;
            }


            // ii) Compute g and h for successorii) else, compute both g and h for successor
            //          successor.g = q.g + distance between
            //                              successor and q
            //          successor.h = distance from goal to
            //          successor
            gScore = openList.top().first + calculateDistance(*current, *successor);
            hScore = calculateDistance(*successor, *end);
            fScore = gScore + hScore;

            successor->f = fScore;
            //  iii) if a node with the same position as
            //            successor is in the OPEN list which has a
            //           lower f than successor, skip this successor
            //Not sure how to implemenet this correctly
            if (closedList.find(successor) != closedList.end() && fScore >= successor->f) {
                continue;
            }
            // iv) Skip this successor if a node with the same position in the CLOSED list has a lower f
            if (closedList.find(successor) != closedList.end() && fScore >= successor->f) {
                continue;
            }


            // Add the node to the open list
            openList.push({fScore, successor});
            successor->parent = current;
        }

        // Step 3e
        closedList.insert(current);
    }

    // If the open list becomes empty and the destination is not reached
    cout <<"Destination is not reachable with these parameters " << endl;
    return result;
}

vector<CampData> numCampsAlgo(map<string, CampData> &camps, const string& oriCity, const string& destCity) {

    double totalDistance = calculateDistance(camps[oriCity], camps[destCity]);
    cout << "Total Distance: " << totalDistance << endl;
    vector<CampData> route;
    std::regex numbersOnly("^[0-9]+$");
    string input;
    cout << "How many of the " << camps.size()-2 << " campsites would you like to visit?" << endl;
    getline(cin, input);
    int numCamps = stoi(input);
    if (numCamps > camps.size()-2) {
        cout << "Too many campsites :(" << endl;
        return route;
    }
    if (regex_match(input, numbersOnly)) {
        int numCamps = stoi(input);
    }
    else {
        cout << "That's not a number unfortunately :(" << endl;
        return route;
    }

    //cout << "How far are you willing to drive between campsites?" << endl;
    //getline(cin, input);
    createNeighbors(camps);
    route.resize(numCamps+2);
    route[0] = camps[oriCity];

    double avgDistance = totalDistance/numCamps;
    cout << "Total Distance: " << totalDistance << endl;
    cout << "Average Distance: " << avgDistance << endl;
    cout << "Neighbors created" << endl;

    map<string, double> dist2dest;
    auto it = camps.begin();
    while (it != camps.end()) {
        string node = it->second.camp;
        double dist = calculateDistance(it->second, camps[destCity]);
        dist2dest.emplace(node, dist);
        it++;
    }
    cout << "Distances calcultated." << endl;

    CampData curr;
    double comparator;
    double currDist;
    double bestPath;
    for (int i = 0; i < route.size()-3; i++) {
        bestPath = 7000;
        for (int j = 0; j < route[i].neighbors.size(); j++) {
            if (route[i].neighbors[j]->camp == curr.camp || route[i].neighbors[j]->camp == route[0].camp) {
                continue;
            }
            else {
                currDist = dist2dest[route[i].neighbors[j]->camp];
                comparator = abs(currDist - avgDistance);
                if (comparator < bestPath) {
                    bestPath = comparator;
                    curr = it->second;
                }
            }
        }
        cout << "Adding to the route." << endl;
        route[i+1] = curr;
    }

    return route;

}


int main() {
    string input;
    cout << "Where would you like to start your camping trip from? \"City, State\"" << endl;
    getline(cin, input);
    istringstream isa(input);
    string oriCity;
    string oriState;
    getline(isa, oriCity, ',');
    getline(isa, oriState);
    oriState = oriState.substr(1, oriState.length()-1);
    cout << "Where would you like your final destination to be? \"City, State\"" << endl;
    getline(cin, input);
    istringstream isb(input);
    string destCity;
    string destState;
    getline(isb, destCity, ',');
    getline(isb, destState);
    destState = destState.substr(1, destState.length()-1);

    int distanceThreshold = 600;
    std::regex numbersOnly("^[0-9]+$");
    cout << "Whats the farthest you're willing to drive between campsites?" << endl;
    getline(cin, input);
    if (regex_match(input, numbersOnly)) {
        distanceThreshold = stoi(input);
    }
    else {
        cout << "That's not a number unfortunately :(" << endl;
        return 1;
    }

    ifstream cityCSV("cities.csv");

    if (!cityCSV.is_open()) {
        cerr << "Error: Unable to open the file \"cities.csv\"" << endl;
        return 1;
    }

    string line;
    getline(cityCSV, input);

    string search, trash, oriLo, oriLa, desLo, desLa;
    CampData destination, origin;

    while (getline(cityCSV, line)) {
        istringstream isc(line);
        getline(isc, input, ',');
        if (input == oriCity) {
            getline(isc, input, ',');
            if (input == oriState) {
                getline(isc, input, ',');
                getline(isc, input, ',');
                origin.lat = stod(input);
                getline(isc, input, ',');
                origin.lon = stod(input);
            }
            else {
                cout << "Check the spelling on the origin state or try another city." << endl;
                return 1;
            }
        }
        if (input == destCity) {
            getline(isc, input, ',');
            if (input == destState) {
                getline(isc, input, ',');
                getline(isc, input, ',');
                destination.lat = stod(input);
                getline(isc, desLo, ',');
                destination.lon = stod(input);
            }
            else {
                cout << "Check the spelling on the destination state or try another city." << endl;
                return 1;
            }
        }
    }

    map<string, CampData> campsites;
    campsites.insert(pair<string, CampData> (oriCity, origin));
    campsites.insert(pair<string, CampData> (destCity, destination));

    bool showersb, toiletsb, petsb, cheapb, reserveb, hookupsb, sanib, waterb = false;

    cout << "Would you like your campsites to have showers? (type 'y' if yes)" << endl;
    getline(cin, input);
    if (input == "y" || input == "Y") {
        showersb = true;
    }

    cout << "Would you like your campsites to have toilets? (type 'y' if yes)" << endl;
    getline(cin, input);
    if (input == "y" || input == "Y") {
        toiletsb = true;
    }

    cout << "Would you like your campsites to allow pets? (type 'y' if yes)" << endl;
    getline(cin, input);
    if (input == "y" || input == "Y") {
        petsb = true;
    }

    cout << "Would you like your campsites to be cheap? (type 'y' if yes)" << endl;
    getline(cin, input);
    if (input == "y" || input == "Y") {
        cheapb = true;
    }

    cout << "Would you like your campsites to allow reservations? (type 'y' if yes)" << endl;
    getline(cin, input);
    if (input == "y" || input == "Y") {
        reserveb = true;
    }

    cout << "Would you like your campsites to have RV Hookups? (type 'y' if yes)" << endl;
    getline(cin, input);
    if (input == "y" || input == "Y") {
        hookupsb = true;
    }

    cout << "Would you like your campsites to have Sanitary Dumps? (type 'y' if yes)" << endl;
    getline(cin, input);
    if (input == "y" || input == "Y") {
        sanib = true;
    }

    cout << "Would you like your campsites to have Drinking Water? (type 'y' if yes)" << endl;
    getline(cin, input);
    if (input == "y" || input == "Y") {
        waterb = true;
    }

    cout << "...One moment while the map populates..." << endl;

    // Open the CSV file
    ifstream file("camp.csv");

    // Check if the file is open
    if (!file.is_open()) {
        cerr << "Error: Unable to open the campsite database" << endl;
        return 1;
    }
    // Read and discard the header line
    getline(file, input);

    // Creating variables to hold input before they're converted to int and bool
    string name, latitude, longitude, confirm, reserve, sani, num_sites, rv, vc, water, ID, shower, pets, fee;

    // Read each line from the file
    while (getline(file, line)) {
        istringstream iss(line);
        CampData camp;
        // Read each column from the line using getline
        getline(iss, latitude, ',');
        camp.lat = stod(latitude);
        getline(iss, longitude, ',');
        camp.lon = stod(longitude);
        getline(iss, camp.camp, ',');
        getline(iss, camp.state, ',');
        getline(iss, name, ',');
        camp.name = name;
        getline(iss, camp.type, ',');
        getline(iss, camp.phone, ',');
        getline(iss, num_sites, ',');
        if (num_sites.length() != 0 && num_sites != "dispersed") {
            camp.sites = stoi(num_sites);
        }
        getline(iss, rv, ',');
        if (rv == "E" || rv == "WE" || rv == "WES") {
            camp.hookups = true;
        }
        else if (hookupsb) {
            camp.approved = false;
        }
        getline(iss, vc, ',');
        if (vc == "FT" || vc == "VT" || vc == "FTVT" || vc == "PT") {
            camp.toilets = true;
        }
        else if (toiletsb) {
            camp.approved = false;
        }
        getline(iss, water, ',');
        if (water == "DW") {
            camp.water = true;
        }
        else if (waterb) {
            camp.approved = false;
        }
        getline(iss, camp.url, ',');
        getline(iss, ID, ',');
        camp.camp_id = stoi(ID);
        getline(iss, confirm, ',');
        if (confirm == "1") {
            camp.url_confirmed = true;
        }
        getline(iss, reserve, ',');
        if (reserve == "RS") {
            camp.reservations = true;
        }
        else if (reserveb) {
            camp.approved = false;
        }
        getline(iss, camp.town, ',');
        getline(iss, sani, ',');
        if (sani == "DP") {
            camp.dump = true;
        }
        else if (sanib) {
            camp.approved = false;
        }
        getline(iss, shower, ',');
        if (shower == "SH") {
            camp.showers = true;
        }
        else if (showersb) {
            camp.approved = false;
        }
        getline(iss, pets, ',');
        if (pets == "PA") {
            camp.pets = true;
        }
        else if (petsb) {
            camp.approved = false;
        }
        getline(iss, fee, ',');
        if (!fee.empty()) {
            camp.cheap = true;
        }
        else if (cheapb) {
            camp.approved = false;
        }

        // If the data matches requirement, add to map.
        if (camp.approved) {
            campsites.insert(pair<string, CampData>(ID, camp));
        }
    }

    cout << campsites.size()-2 <<" campsites found." << endl;

    cout << "Would you like to find your campsites using Yen's Algorithm or Bidirectional Search? type \"1\" or \"2\"" << endl;
    getline(cin, input);
    vector<CampData> camps;
    if (input == "1") {
        camps = DijkstrasAlgo(campsites, oriCity, destCity);
    }
    else if (input == "2") {
        camps = AStar(campsites, oriCity, destCity);
    }
    else if (input == "3") {
        auto it = campsites.begin();
        for (int i = 0; i<campsites.size(); i++) {
            camps.push_back(it->second);
            it++;
        }
    }
    else if (input == "4") {
        camps = numCampsAlgo(campsites, oriCity, destCity);
    }
    else if (input == "5") {
        //camps = willAlgo(campsites, oriCity, destCity);
    }
    else {
        cout << "ur a bum for not putting a good number" << endl;
        return 1;
    }

    cout << endl;
    cout << "Here's your itinerary!" << endl;
    cout << "After leaving from " << oriCity << ", you'll travel << " << calculateDistance(camps[0], camps[1]) << " kilometers to " << camps[1].camp << endl;
    for (int i = 1; i < camps.size()-1; i++) {
        cout << "Campsite #" << i << ": " << camps[i].camp << " in " << camps[i].town << ", " << camps[i].state << endl;
        cout << "# of campsites: " << camps[i].sites << endl;
        cout << "These campsites:" << endl;
        if (camps[i].cheap) {
            cout << "Likely cheap :D" << endl;
        }
        else {
            cout << "Could be expensive :(" << endl;
        }
        if (camps[i].dump) {
            cout << "Has a sanitary dump :D" << endl;
        }
        else {
            cout << "No sanitary dump :(" << endl;
        }
        if (camps[i].hookups) {
            cout << "RV Hookups available" << endl;
        }
        else {
            cout << "No RV Hookups" << endl;
        }
        if (camps[i].pets) {
            cout  << "Pets allowed :D" << endl;
        }
        else {
            cout << "No pets allowed >:(" << endl;
        }
        if (camps[i].reservations) {
            cout << "Reservations allowed" << endl;
        }
        else {
            cout << "No reservations" << endl;
        }
        if (camps[i].showers) {
            cout << "Showers available :D" << endl;
        }
        else {
            cout << "No showers :(" << endl;
        }
        if (camps[i].toilets) {
            cout << "Toilets available :D" << endl;
        }
        else {
            cout << "No toilets..." << endl;
        }
        if (camps[i].water) {
            cout << "Drinking water available" << endl;
        }
        else {
            cout << "No drinking water..." << endl;
        }
        cout << "Website: " << camps[i].url;
        if (camps[i].url_confirmed) {
            cout << " (confirmed)" << endl;
        }
        else {
            cout << " (unconfirmed)" << endl;
        }
        cout << "Phone number: " << camps[i].phone << endl;
        cout << "After leaving from " << camps[i].camp << " you'll travel " << calculateDistance(camps[i], camps[i+1]) << "kilometers to " << camps[i+1].camp << endl;
        cout << endl;
    }
    cout << "Welcome to " << destCity << endl;
    cout << "There's your trip! Happy Camping :)" << endl;
    return 1;
}