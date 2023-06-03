#include<iostream>
#include<fstream>
#include<string>
#include<unordered_map>
#include<memory>
#include<vector>
#include<stack>
#include<queue>
#include<thread>
#include<algorithm>

#include<athread.h>

using ADDR_MAP_NAME = std::unordered_map<std::string, std::pair<std::string, std::string>>;

void parseAddrToMap(const std::string filename, ADDR_MAP_NAME& addrMapName);

using ULL = unsigned long long;

typedef struct ResultNodeInfo {
    std::string funcNameAddr;
    ULL cpuTime;
    std::string cpuTimePercentage;
    ULL selfTime;
    std::string selfTimePercentage;
    ULL callTimes;
    double avaSelfTime;
    ResultNodeInfo() : cpuTime(0), selfTime(0), callTimes(0) {}
}ResultNodeInfo;

void parseTraceResToVec(const std::string filename, std::vector<ResultNodeInfo*>& resNodeInfoVec);

void printResultNodeInfo(const std::vector<ResultNodeInfo*>& resNodeInfoVec);

bool cmpCputime(ResultNodeInfo* a, ResultNodeInfo* b);
bool cmpSelftime(ResultNodeInfo* a, ResultNodeInfo* b);
bool cmpCalltimes(ResultNodeInfo* a, ResultNodeInfo* b);
bool cmpAvaSelftime(ResultNodeInfo* a, ResultNodeInfo* b);

void writeSortResToFile(const std::vector<ResultNodeInfo*>& resNodeInfoVec, ADDR_MAP_NAME& addrMapName, const std::string filename);

ULL getCpuClockCycle();

int main(int argc, char** argv) {

    if (argc != 3) {
        std::cout << "ERROR: args must be 3" << std::endl;
        return -1;
    }

    std::string parseAddrFile   {argv[1]};
    std::string traceResultFile {argv[2]};

    ADDR_MAP_NAME addrMapName;
    parseAddrToMap(parseAddrFile, addrMapName);

    std::vector<ResultNodeInfo*> resNodeInfoVec;
    parseTraceResToVec(traceResultFile, resNodeInfoVec);

    // std::sort(resNodeInfoVec.begin(), resNodeInfoVec.end(), cmpCputime);
    // printResultNodeInfo(resNodeInfoVec);

    std::sort(resNodeInfoVec.begin(), resNodeInfoVec.end(), cmpCputime);
    writeSortResToFile(resNodeInfoVec, addrMapName, traceResultFile + ".cputimeSort");

    std::sort(resNodeInfoVec.begin(), resNodeInfoVec.end(), cmpSelftime);
    writeSortResToFile(resNodeInfoVec, addrMapName, traceResultFile + ".selftimeSort");

    std::sort(resNodeInfoVec.begin(), resNodeInfoVec.end(), cmpCalltimes);
    writeSortResToFile(resNodeInfoVec, addrMapName, traceResultFile + ".calltimesSort");

    std::sort(resNodeInfoVec.begin(), resNodeInfoVec.end(), cmpAvaSelftime);
    writeSortResToFile(resNodeInfoVec, addrMapName, traceResultFile + ".avaselftimeSort");

    return 0;
}



void parseAddrToMap(const std::string filename, ADDR_MAP_NAME& addrMapName)
{
    std::ifstream fin;
    fin.open(filename, std::ios::in);
    if (!fin.is_open()) {
        std::cerr << "open file " << filename << " Error" << std::endl;
        return;
    }
    std::string s1, s2, s3;
    while(fin >> s1 >> s2 >> s3) {
        addrMapName.emplace(s1, std::make_pair(s2, s3));
    }
    std::cout << "parseAddrToMap Done!" << std::endl;
}

void parseTraceResToVec(const std::string filename, std::vector<ResultNodeInfo*>& resNodeInfoVec)
{
    std::ifstream fin;
    fin.open(filename, std::ios::in);
    if (!fin.is_open()) {
        std::cerr << "open file " << filename << " Error" << std::endl;
        return;
    }

    std::string funaddr, cputime, percent1, selftime, percent2, calltimes, avaselftime;
    ResultNodeInfo* NodePtr = nullptr;
    fin >> funaddr >> cputime >> percent1 >> selftime >> percent2 >> calltimes >> avaselftime;
    while (fin >> funaddr >> cputime >> percent1 >> selftime >> percent2 >> calltimes >> avaselftime) {
        NodePtr = new ResultNodeInfo;
        NodePtr->funcNameAddr = funaddr;
        NodePtr->cpuTime = std::stoull(cputime);
        NodePtr->cpuTimePercentage = percent1;
        NodePtr->selfTime = std::stoull(selftime);
        NodePtr->selfTimePercentage = percent2;
        NodePtr->callTimes = std::stoull(calltimes);
        NodePtr->avaSelfTime = std::stod(avaselftime);
        resNodeInfoVec.push_back(NodePtr);
    }
    std::cout << "parseTraceResToVec Done !" << std::endl; 
}

void printResultNodeInfo(const std::vector<ResultNodeInfo*>& resNodeInfoVec)
{
    for (const auto& item : resNodeInfoVec) {
        printf("%s\t", item->funcNameAddr.c_str());
        printf("%lld\t", item->cpuTime);
        printf("%s\t", item->cpuTimePercentage.c_str());
        printf("%lld\t", item->selfTime);
        printf("%s\t", item->selfTimePercentage.c_str());
        printf("%lld\t", item->callTimes);
        printf("%lf\n", item->avaSelfTime);
    }
}

bool cmpCputime(ResultNodeInfo* a, ResultNodeInfo* b)
{
    return a->cpuTime > b->cpuTime;
}
bool cmpSelftime(ResultNodeInfo* a, ResultNodeInfo* b)
{
    return a->selfTime > b->selfTime;
}
bool cmpCalltimes(ResultNodeInfo* a, ResultNodeInfo* b)
{
    return a->callTimes > b->callTimes;
}
bool cmpAvaSelftime(ResultNodeInfo* a, ResultNodeInfo* b)
{
    return a->avaSelfTime > b->avaSelfTime;
}


void writeSortResToFile(const std::vector<ResultNodeInfo*>& resNodeInfoVec, ADDR_MAP_NAME& addrMapName, const std::string filename)
{
    FILE* fp = fopen(filename.c_str(), "w");
    if (fp == NULL) {
        printf("ERROR: Failed to open file %s\n", filename.c_str());
        return;
    }
    fprintf(fp, "***************************函数采样信息*****************************\n");
    fprintf(fp, "%-50s\t%-18s\t%-15s\t%-18s\t%-15s\t%-15s\t%-15s\n", "FuncName", "CpuTime", "Percentage", "SelfTime", "Percentage", "CallTimes", "AvarageSelfTime");
    for (const auto& item : resNodeInfoVec) {
        fprintf(fp, "%-50s\t", addrMapName[item->funcNameAddr].first.c_str());
        fprintf(fp, "%-18lld\t", item->cpuTime);
        fprintf(fp, "%-15s\t", item->cpuTimePercentage.c_str());
        fprintf(fp, "%-18lld\t", item->selfTime);
        fprintf(fp, "%-15s\t", item->selfTimePercentage.c_str());
        fprintf(fp, "%-15lld\t", item->callTimes);
        fprintf(fp, "%-15lf\n", item->avaSelfTime);
    }

    fprintf(fp, "\n\n\n");
    fprintf(fp, "***************************CPU时钟周期数****************************\n");
    ULL sumClock = 0, tmp = 0;
    for (int i = 0; i < 10; i++) {
        tmp = getCpuClockCycle();
        fprintf(fp, "[%dtimes] Test CPU Clock Cycle:\t %lld\n", i, tmp);
        sumClock += tmp;
    }
    fprintf(fp, "\nAvarage CPU Clock Cycle is %lld\n", sumClock / 10);

    fprintf(fp, "\n\n\n");
    fprintf(fp, "***************************函数文件路径*****************************\n");

    for (const auto& item : resNodeInfoVec) {
        fprintf(fp, "%-50s\t", addrMapName[item->funcNameAddr].first.c_str());
        fprintf(fp, "%-50s\n", addrMapName[item->funcNameAddr].second.c_str());
    }
    fclose(fp);
}


ULL getCpuClockCycle()
{
    ULL t1 = athread_time_cycle();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ULL t2 = athread_time_cycle();
    return t2 - t1;
}