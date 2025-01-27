#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <set>
#include <unordered_set>

void printCurrentTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    struct tm time_info;
    localtime_s(&time_info, &now_time);

    std::cout << "Current Time: " << std::put_time(&time_info, "%Y-%m-%d %H:%M:%S") << std::endl;
}

std::vector<std::string> extractVariables(const std::string& inputFile) {
    std::ifstream in(inputFile);
    if (!in.is_open()) {
        std::cerr << "Failed to open input file for variable extraction: " << inputFile << std::endl;
        return {};
    }

    std::vector<std::string> variables;
    std::string line;
    while (std::getline(in, line)) {
        if (line.find("<code>") != std::string::npos) {
            size_t start = line.find("<code>") + 6; // Length of "<code>"
            size_t end = line.find("</code>");
            variables.push_back(line.substr(start, end - start));
        }
    }

    in.close();
    return variables;
}

void processXMLAsText(const std::string& inputFile, const std::string& outputFile, double pThreshold, const std::vector<std::string>& userOrder) {
    std::ifstream in(inputFile);
    if (!in.is_open()) {
        std::cerr << "Failed to open input file: " << inputFile << std::endl;
        return;
    }

    std::ofstream out(outputFile);
    if (!out.is_open()) {
        std::cerr << "Failed to open output file: " << outputFile << std::endl;
        return;
    }

    // Use extractVariables to get variables from the XML file
    std::vector<std::string> variables = extractVariables(inputFile);
    std::unordered_set<std::string> selectedVariables(userOrder.begin(), userOrder.end());
    std::vector<int> variableIndices;

    // Match the user-specified order to the variable indices
    for (const auto& var : userOrder) {
        auto it = std::find(variables.begin(), variables.end(), var);
        if (it != variables.end()) {
            variableIndices.push_back(std::distance(variables.begin(), it));
        }
    }

    // Write the header dynamically based on user-specified order
    out << "Timestep,DateTime,Particle_ID";
    for (const auto& var : userOrder) {
        if (selectedVariables.count(var)) {
            out << "," << var;
        }
    }
    out << "\n";

    std::string currentTimestep, currentDateTime;
    int nextTimestepPrint = 0;

    std::string line;
    while (std::getline(in, line)) {
        if (line.find("<TimeStep") != std::string::npos && line.find("nr=") != std::string::npos) {
            size_t pos = line.find("nr=\"");
            if (pos != std::string::npos) {
                size_t start = pos + 4;
                size_t end = line.find("\"", start);
                currentTimestep = line.substr(start, end - start);
            }
        }

        if (currentTimestep != "" && std::stoi(currentTimestep) >= nextTimestepPrint) {
            printCurrentTime();
            std::cout << "Processed " << currentTimestep << " timesteps..." << std::endl;
            nextTimestepPrint += 100;
        }

        if (line.find("<DateTime>") != std::string::npos) {
            size_t start = line.find("<DateTime>") + 10;
            size_t end = line.find("</DateTime>");
            currentDateTime = line.substr(start, end - start);
        }

        if (line.find("<Particle") != std::string::npos && line.find("Nr=") != std::string::npos) {
            std::string particleID;
            size_t pos = line.find("Nr=\"");
            if (pos != std::string::npos) {
                size_t start = pos + 4;
                size_t end = line.find("\"", start);
                particleID = line.substr(start, end - start);
            }

            std::vector<double> cdata(variables.size(), 0);

            if (std::getline(in, line) && line.find("<![CDATA[") != std::string::npos) {
                size_t start = line.find("<![CDATA[") + 9;
                size_t end = line.find("]]>", start);
                std::string cdataText = line.substr(start, end - start);

                size_t pos = 0;
                int index = 0;
                while ((pos = cdataText.find(',')) != std::string::npos && index < variables.size()) {
                    cdata[index++] = std::stod(cdataText.substr(0, pos));
                    cdataText.erase(0, pos + 1);
                }
                if (!cdataText.empty() && index < variables.size()) {
                    cdata[index] = std::stod(cdataText);
                }

                if (cdata[variableIndices.back()] > pThreshold) {
                    out << currentTimestep << "," << currentDateTime << "," << particleID;
                    for (const auto& idx : variableIndices) {
                        out << "," << cdata[idx];
                    }
                    out << "\n";
                }
            }
        }
    }

    in.close();
    out.close();
    std::cout << "Processing complete. Filtered data written to: " << outputFile << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <inputFile> <outputFile> <threshold> [variables...]" << std::endl;
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];
    double pThreshold = std::stod(argv[3]);

    std::vector<std::string> selectedVariables;
    for (int i = 4; i < argc; ++i) {
        selectedVariables.push_back(argv[i]);
    }

    processXMLAsText(inputFile, outputFile, pThreshold, selectedVariables);
    return 0;
}
