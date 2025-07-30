#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <filesystem>
#include <cctype>
#include <cstdlib>
#include <ctime>
using namespace std;
namespace fs = std::filesystem;

struct Song {
    string artist;
    string title;
};

string normalize(const string& s) {
    string out;
    for (char c : s) {
        if (isalnum(static_cast<unsigned char>(c)) || c == ' ') {
            out += tolower(static_cast<unsigned char>(c));
        }
    }
    size_t firstAlpha = out.find_first_of("abcdefghijklmnopqrstuvwxyz");
    if (firstAlpha != string::npos) {
        out = out.substr(firstAlpha);
    } else {
        out = "zzz" + out;
    }
    return out;
}

vector<Song> loadSongs(const string& filename) {
    vector<Song> songs;
    fs::path current = fs::current_path();
    while (!fs::exists(current / "resources") && current.has_parent_path()) {
        current = current.parent_path();
    }
    fs::path csvPath = current / "resources" / filename;
    ifstream file(csvPath);
    if (!file.is_open()) {
        cerr << "Failed to open CSV at: " << csvPath << endl;
        return songs;
    }
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string artist, title;
        if (getline(ss, artist, ',') && getline(ss, title)) {
            songs.push_back({artist, title});
        }
    }
    cout << "CSV loaded from: " << csvPath << endl;
    return songs;
}

int partition(vector<Song>& songs, int low, int high, bool byTitle) {
    int randomIndex = low + rand() % (high - low + 1);
    swap(songs[randomIndex], songs[high]);
    string pivot = byTitle ? normalize(songs[high].title) : normalize(songs[high].artist);
    int i = low - 1;
    for (int j = low; j < high; j++) {
        string val = byTitle ? normalize(songs[j].title) : normalize(songs[j].artist);
        if (val <= pivot) {
            i++;
            swap(songs[i], songs[j]);
        }
    }
    swap(songs[i+1], songs[high]);
    return i+1;
}

void quickSort(vector<Song>& songs, int low, int high, bool byTitle) {
    while (low < high) {
        int pi = partition(songs, low, high, byTitle);
        if (pi - low < high - pi) {
            quickSort(songs, low, pi - 1, byTitle);
            low = pi + 1;
        } else {
            quickSort(songs, pi + 1, high, byTitle);
            high = pi - 1;
        }
    }
}

void merge(vector<Song>& songs, int l, int m, int r, bool byTitle) {
    int n1 = m-l+1, n2 = r-m;
    vector<Song> L(n1), R(n2);
    for (int i=0; i<n1; i++) L[i] = songs[l+i];
    for (int j=0; j<n2; j++) R[j] = songs[m+1+j];
    int i=0, j=0, k=l;
    while (i<n1 && j<n2) {
        string leftVal = byTitle ? normalize(L[i].title) : normalize(L[i].artist);
        string rightVal = byTitle ? normalize(R[j].title) : normalize(R[j].artist);
        if (leftVal <= rightVal) songs[k++] = L[i++];
        else songs[k++] = R[j++];
    }
    while (i<n1) songs[k++] = L[i++];
    while (j<n2) songs[k++] = R[j++];
}

void mergeSort(vector<Song>& songs, int l, int r, bool byTitle) {
    if (l < r) {
        int m = l + (r-l)/2;
        mergeSort(songs, l, m, byTitle);
        mergeSort(songs, m+1, r, byTitle);
        merge(songs, l, m, r, byTitle);
    }
}

int main() {
    srand(static_cast<unsigned>(time(0)));
    vector<Song> songs = loadSongs("songdata.csv");
    cout << "Loaded " << songs.size() << " songs\n";
    if (songs.empty()) return 0;
    cout << "Sort by:\n1. Title\n2. Artist\nChoose option: ";
    int choice;
    cin >> choice;
    bool byTitle = (choice == 1);
    auto quickVec = songs;
    auto mergeVec = songs;
    auto start = chrono::high_resolution_clock::now();
    quickSort(quickVec, 0, quickVec.size()-1, byTitle);
    auto end = chrono::high_resolution_clock::now();
    auto quickTime = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    start = chrono::high_resolution_clock::now();
    mergeSort(mergeVec, 0, mergeVec.size()-1, byTitle);
    end = chrono::high_resolution_clock::now();
    auto mergeTime = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    cout << "\nQuicksort Time: " << quickTime << " ms\n";
    cout << "Merge Sort Time: " << mergeTime << " ms\n\n";
    cout << "Top 10 sorted by " << (byTitle ? "title" : "artist") << ":\n";
    for (int i=0; i<10 && i<quickVec.size(); i++) {
        cout << quickVec[i].artist << " - " << quickVec[i].title << "\n";
    }
    return 0;
}
