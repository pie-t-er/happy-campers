#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <queue>
#include <set>

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
    int sites = 0;
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

};

// straight ripped this equation from chat gpt, should be more accurate than the previous one
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
    dist = 6371 * 2 * atan2(sqrt(a), sqrt(1 - a));
    return dist;
}

// this function takes in the map and the diatance threshold and fills the neighbors vector with neighbors within the
// distance threshold
void createNeighbors(double distanceThreshold, map<string, CampData> &camps) {
    //loop through every node and calculate if its long and lat is within neighbor distance of the other node
    for (auto it = camps.begin(); it != camps.end(); it++) {
        for (auto jt = next(it); jt != camps.end(); jt++) {
            // these^ loop should optimize this function to not loop through every campsite multiple times
            double distance = calculateDistance(it->second, jt->second);
            if (distance < distanceThreshold) {
                // add both as neighbors only once to each other since every neighbor also has the node as a neighbor
                it->second.neighbors.push_back(&jt->second);
                jt->second.neighbors.push_back(&it->second);
            }
        }
    }
}

// ADDED THIS TO BE USED IN YENS ALGORITHM //

struct CompareDist {
    bool operator()(const pair<double, CampData*>& a, const pair<double, CampData*>& b) {
        return a.first > b.first;
    }
};

// YENS ALGORITHM //
vector<CampData> YensAlgo(int numNodes, map<string, CampData> &camps, const string& oriCity, const string& destCity) {
    // result vector is to store the path //
    vector<CampData> result;
    priority_queue<pair<double,CampData*>, vector<pair<double,CampData*>>, CompareDist> candidates;
    set<CampData*> visited;

    // origin city will be a part of the given map, without any campsite data, just the starting location.
    auto itStart = camps.find(oriCity);

    if (itStart == camps.end()) {
        cerr << "Error: Origin city missing from map." << endl;
        return result;
    }

    // destination city will be a part of the given map, without any campsite data, just the ending location.
    auto itEnd = camps.find(destCity);

    if (itEnd == camps.end()) {
        cerr << "Error: Destination city missing from map." << endl;
        return result;
    }

    CampData* start = &itStart->second;
    CampData* end = &itEnd->second;

    candidates.push({0.0, start});


    while(!candidates.empty()){
        CampData* temp = candidates.top().second;
        double currentDistance = candidates.top().first;
        candidates.pop();

        if(temp == end){
            result.push_back(*temp);
            break;
        }
        if(visited.find(temp) == visited.end()){
            visited.insert(temp);
            result.push_back(*temp);
            // loops through neighbors
            for(CampData* neighbor: temp->neighbors){
                if(visited.find(neighbor) == visited.end()){
                    double potentialDistance = currentDistance + calculateDistance(*temp, *neighbor);
                    candidates.push({potentialDistance, neighbor});

                }
            }
        }
    }
    return result;
}

vector<CampData> BidirectionalSearch(int numNodes, map<string, CampData> &camps, const string& oriCity, const string& destCity) {

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

    ifstream cityCSV("cities.csv");

    if (!cityCSV.is_open()) {
        cerr << "Error: Unable to open the file " << "cities.csv" << endl;
        return 1;
    }

    string line;
    getline(cityCSV, input);

    string search;
    string trash;
    string oriLo;
    string oriLa;
    string desLo;
    string desLa;
    CampData destination;
    CampData origin;

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
                cout << "Check the spelling on the state or try another city." << endl;
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
                cout << "Check the spelling on the state or try another city." << endl;
                return 1;
            }
        }
    }

    map<string, CampData> campsites;
    campsites.insert(pair<string, CampData> (oriCity, origin));
    campsites.insert(pair<string, CampData> (destCity, destination));

    bool showersb = false;
    bool toiletsb = false;
    bool petsb = false;
    bool cheapb = false;
    bool reserveb = false;
    bool hookupsb = false;
    bool sanib = false;
    bool waterb = false;

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

    bool testing = true;
    while (testing) {
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
        string line;
        string name;
        string latitude;
        string longitude;
        string confirm;
        string reserve;
        string sani;
        string num_sites;
        string rv;
        string vc;
        string water;
        string ID;
        string shower;
        string pets;
        string fee;

        // Read each line from the file
        while (getline(file, line)) {
            istringstream iss(line);
            CampData camp;
            // Read each column from the line using getline
            getline(iss, latitude, ',');
            getline(iss, longitude, ',');
            camp.lat = stod(latitude);
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
        // here's a test:
        createNeighbors(double(1000), campsites);
        std::cout << "Test" << std::endl;
        // set a breakpoint ^here and debug to verify the campsites are filled with the right neighbors, should be a quick
        // runtime with distanceThreshold set to 25.
        testing = false;
    }

    cout << "How many of the "<< campsites.size()-2 <<" campsites would you like to visit?" << endl;
    getline(cin, input);
    int numCampsites = stoi(input);
    if (numCampsites > campsites.size()-2) {
        cout << "We can't find that many campsites that fit your criteria :(" << endl;
        return 1;
    }

    cout << "Would you like to find your campsites using Yen's Algorithm or Bidirectional Search? type \"1\" or \"2\"" << endl;
    getline(cin, input);
    vector<CampData> camps;
    if (input == "1") {
        camps = YensAlgo(numCampsites, campsites, oriCity, destCity);
    }
    else if (input == "2") {
        camps = BidirectionalSearch(numCampsites, campsites, oriCity, destCity);
    }
    else if (input == "3") {
        auto it = campsites.begin();
        for (int i = 0; i<numCampsites+2; i++) {
            camps.push_back(it->second);
            it++;
        }
    }
    else {
        cout << "ur a bum for not putting a good number" << endl;
        return 1;
    }

    cout << endl;
    cout << "Here's your itinerary!" << endl;
    cout << "After leaving from " << oriCity << ", you'll travel to " << camps[1].camp << endl;
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
        cout << "After leaving from " << camps[i].camp << " you'll travel to " << camps[i+1].camp << endl;
        cout << endl;
    }
    cout << "There's your trip! Happy Camping :)" << endl;
    return 1;
}
