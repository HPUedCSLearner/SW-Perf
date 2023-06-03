#include<iostream>
#include<fstream>
#include<string>
#include<unordered_map>
#include<memory>
#include<vector>
#include<stack>
#include<queue>
#include<thread>

#include<athread.h>

#include<assert.h>
#include<stdio.h>

const std::string enterFlag {"ENTER"};
const std::string exitFlag  {"EXIT"};
const std::string depthFlag {"Depth"};
const std::string costFlag  {"ConstTime"};

using ULL = unsigned long long;

// 采样结果文件，也是非常丰富，可以聚合调用关系，可视化......
// node 数据非常丰富，还没有完全用完
typedef struct NODE
{
    std::string funcAddr;
    std::string funcName;
    ULL cpuTime;
    ULL selfTime;
    ULL sonNum;
    std::vector<struct NODE*> son;
    NODE() : cpuTime(0), selfTime(0), sonNum(0) {}
}NODE, *NODE_PTR;

NODE_PTR root = nullptr;
std::stack<NODE_PTR> preNodeStk;
NODE_PTR curNode = nullptr;
NODE_PTR preNode = nullptr;


std::queue<NODE_PTR> que;

void printSplitLine();
void printTree(NODE_PTR root);
void calSelfTime(NODE_PTR root);

// 统计聚合节点，只关心函数地址，cpuTime, selfTime, callTime
typedef struct STATIS_NODE
{
    std::string funcAddr;
    ULL callTimes;
    ULL selfTime;
    ULL cpuTime;
    STATIS_NODE() : callTimes(0), selfTime(0), cpuTime(0) {}
}STATIS_NODE, *STATIS_NODE_PTR;
std::unordered_map<std::string, STATIS_NODE_PTR> funNodeMap;
STATIS_NODE_PTR curStaNodePtr = nullptr;
void statisticNode(NODE_PTR root, std::unordered_map<std::string, STATIS_NODE*> &funcNodeMap);
void printFuncNodeMap(const std::unordered_map<std::string, STATIS_NODE*> &funcNodeMap);

ULL allTime = 0;

void freeTreeNode(NODE_PTR root);
void freeMapNode(std::unordered_map<std::string, STATIS_NODE*> &funcNodeMap);

static FILE *fp = NULL;
void printFuncNodeMapToFile(const std::unordered_map<std::string, STATIS_NODE*> &funcNodeMap, const std::string& fileName);
void writeFuncAddrToFIle(const std::unordered_map<std::string, STATIS_NODE*> &funcNodeMap, const std::string& fileName);

ULL cpuClock = 0;
ULL getCpuClockCycle();

// std::vector<std::string> addrInfo;

// typedef struct RESULT_NODE_INFO
// {
//     std::string funcAddr;
//     ULL cpuTime;
//     std::string percentCupTime;
//     ULL selfTime;
//     std::string percentSelfTime;
//     ULL callTimes;
//     double avarageSelfTime;
// }RESULT_NODE_INFO;
// std::vector<RESULT_NODE_INFO> resultNodeVec;

int main(int argc, char** argv)
{
    
    if (argc != 2) {
        std::cout << "ERROR: args must be 2" << std::endl;
        exit(1);
    }

    cpuClock = getCpuClockCycle();
    ULL start, end;

    std::string filename {argv[1]};

    std::ifstream m_fin;
    m_fin.open(filename, std::ios::in);
    if (!m_fin.is_open()) {
        std::cerr << "open file " << filename << "Error" << std::endl;
        return 1;
    }

    // step1 - 建树
    start = athread_time_cycle();
    printf("*******************Step 1 create tree********************\n");
    std::string s1, s2, s3, s4, s5, s6;
    while (m_fin >> s1) {
        // // debuf for read file
        // if (s1 == enterFlag) {
        //     m_fin >> s2 >> s3 >> s4;
        //     std::cout << s1 << " " << s2 << " " << s3 << " " << s4 << " " << std::endl;
        // } else {
        //     m_fin >> s2 >> s3 >> s4 >> s5 >> s6;
        //     std::cout << s1 << " " << s2 << " " << s3 << " " << s4 << " " << s5 << " " << s6 << " " << std::endl;
        // }

        // create func call tree
        if (s1 == enterFlag) {
            m_fin >> s2 >> s3 >> s4;
            if (preNodeStk.empty()) {
                curNode = new NODE;
                preNodeStk.push(curNode);
                root = curNode; // record root
                continue;
            } else {
                curNode = new NODE;
                preNode = preNodeStk.top();
                preNodeStk.push(curNode);
                // 刷新pre-NODE数据
                preNode->son.push_back(curNode);
                preNode->sonNum++;
            }
        } else {
            m_fin >> s2 >> s3 >> s4 >> s5 >> s6;
            // 刷新当前节点的时间 和 函数名字
            curNode = preNodeStk.top();
            curNode->cpuTime = std::stoull(s6);
            curNode->funcAddr = s2;
            // printf("s2 = %s\n", s2.c_str());

            preNodeStk.pop();
        }
    }
    end = athread_time_cycle();
    printf("this step cost %lf ms\n", static_cast<double>(end - start) / cpuClock * 1000);

    // printSplitLine();
    // printTree(root);

    // step2 - 求self时间
    start = athread_time_cycle();
    printf("*******************Step 2 cal self time*******************\n");
    calSelfTime(root);
    allTime = root->cpuTime;
    // printf("allTime %lld\n", allTime);
    // printSplitLine();
    // printTree(root);
    end = athread_time_cycle();
    printf("this step cost %lf ms\n", static_cast<double>(end - start) / cpuClock * 1000);

    // step3 - 聚合函数
    start = athread_time_cycle();
    printf("*******************Step 3 statisitc node info*************\n");
    statisticNode(root, funNodeMap);
    // printSplitLine();
    // printFuncNodeMap(funNodeMap);
    printFuncNodeMapToFile(funNodeMap, filename + ".out");
    end = athread_time_cycle();
    printf("this step cost %lf ms\n", static_cast<double>(end - start) / cpuClock * 1000);
    

    // step4 - 两种热点分析结果
    start = athread_time_cycle();
    printf("*******************Step 4 statisitc node result************\n");
    // printSplitLine();
    writeFuncAddrToFIle(funNodeMap, filename + ".addr");
    end = athread_time_cycle();
    printf("this step cost %lf ms\n", static_cast<double>(end - start) / cpuClock * 1000);


    // 释放空间
    freeTreeNode(root);
    freeMapNode(funNodeMap);

    return 0;
}






// std::queue<NODE_PTR> que;

// BFS
void printTree(NODE_PTR root) 
{
    if (root == nullptr) {
        return;
    }
    // Access
    printf("%s  ", root->funcAddr.c_str());
    printf("\tcpuTime: %lld", root->cpuTime);
    printf("\tselfTime %lld", root->selfTime);
    printf("\n");

    // 入队孩子
    for (ULL i = 0; i < root->sonNum; ++i) {
        que.push(root->son[i]);
    }

    while (!que.empty()) {
        root = que.front();
        que.pop();
        printTree(root);
    }
}

void calSelfTime(NODE_PTR root)
{
    if (root == nullptr) {
        return;
    }
    // Access
    ULL allSonTime = 0;

    // 入队孩子
    for (ULL i = 0; i < root->sonNum; ++i) {
        que.push(root->son[i]);
        allSonTime += root->son[i]->cpuTime;
    }

    root->selfTime = root->cpuTime - allSonTime;

    while (!que.empty()) {
        root = que.front();
        que.pop();
        calSelfTime(root);
    }
}

void printSplitLine()
{
    printf("**************************************************\n");
}

void statisticNode(NODE_PTR root, std::unordered_map<std::string, STATIS_NODE*> &funcNodeMap)
{
    if (root == nullptr) {
        return;
    }
    // Access
    if (funcNodeMap.find(root->funcAddr) == funcNodeMap.end()) {
        // printf("not find in this map\n");
        curStaNodePtr = new STATIS_NODE;
        curStaNodePtr->callTimes++;
        curStaNodePtr->cpuTime += root->cpuTime;
        curStaNodePtr->selfTime += root->selfTime;
        curStaNodePtr->funcAddr = root->funcAddr;  // maybe this line is redundancy
        funcNodeMap.emplace(root->funcAddr, curStaNodePtr);
        // funcNodeMap.insert({root->funcAddr, curStaNodePtr});
    } else {
        // printf("has find in this map\n");
        funcNodeMap[root->funcAddr]->callTimes++;
        funcNodeMap[root->funcAddr]->cpuTime += root->cpuTime;
        funcNodeMap[root->funcAddr]->selfTime += root->selfTime;
    }

    // 入队孩子
    for (ULL i = 0; i < root->sonNum; ++i) {
        que.push(root->son[i]);
    }


    while (!que.empty()) {
        root = que.front();
        que.pop();
        statisticNode(root, funcNodeMap);
    }
}

void printFuncNodeMap(const std::unordered_map<std::string, STATIS_NODE*> &funcNodeMap)
{
    printf("FuncAddr\t\tCpuTime\t\tPercentage\tSelfTime\tPercentage\tCallTimes\tAvarageSelfTime\n");
    for (const auto& item : funcNodeMap) {
        printf("%s\t", item.first.c_str());
        printf("\t%lld", item.second->cpuTime);
        printf("\t\t%lf%%", double(item.second->cpuTime) / allTime * 100);
        printf("\t%lld", item.second->selfTime);
        printf("\t\t%lf%%", double(item.second->selfTime) / allTime * 100);
        printf("\t%lld", item.second->callTimes);
        printf("\t\t%lf", double(item.second->selfTime) / item.second->callTimes);
        printf("\n");
    }
}

void printFuncNodeMapToFile(const std::unordered_map<std::string, STATIS_NODE*> &funcNodeMap, const std::string& fileName)
{
    fp = fopen(fileName.c_str(), "w");
    if (fp == NULL) {
        printf("ERROR: Failed to open file %s\n", fileName.c_str());
        return;
    }
    fprintf(fp, "FuncAddr\t\t\tCpuTime\t\tPercentage\t\tSelfTime\t\tPercentage\t\tCallTimes\t\tAvarageSelfTime\n");
    for (const auto& item : funcNodeMap) {
        fprintf(fp, "%s", item.first.c_str());
        fprintf(fp, "\t\t%lld", item.second->cpuTime);
        fprintf(fp, "\t\t\t\t%lf%%", double(item.second->cpuTime) / allTime * 100);
        fprintf(fp, "\t\t%lld", item.second->selfTime);
        fprintf(fp, "\t\t\t\t%lf%%", double(item.second->selfTime) / allTime * 100);
        fprintf(fp, "\t\t%lld", item.second->callTimes);
        fprintf(fp, "\t\t\t%lf", double(item.second->selfTime) / item.second->callTimes);
        fprintf(fp, "\n");
    }
    printf("statisitc node result out file path %s\n", fileName.c_str());
    fclose(fp);
}

void writeFuncAddrToFIle(const std::unordered_map<std::string, STATIS_NODE*> &funcNodeMap, const std::string& fileName)
{
    fp = fopen(fileName.c_str(), "w");
    if (fp == NULL) {
        printf("ERROR: Failed to open file %s\n", fileName.c_str());
        return;
    }
    for (const auto& item : funcNodeMap) {
        fprintf(fp, "%s\n", item.first.c_str());
    }
    fclose(fp);
}

void freeTreeNode(NODE_PTR root)
{
    if (root == nullptr) {
        return;
    }

    // 入队孩子
    for (ULL i = 0; i < root->sonNum; ++i) {
        que.push(root->son[i]);
    }

    // Access Free
    delete root;
    root = nullptr;

    while (!que.empty()) {
        root = que.front();
        que.pop();
        freeTreeNode(root);
    }
}

void freeMapNode(std::unordered_map<std::string, STATIS_NODE*> &funcNodeMap)
{
    for (auto& item : funcNodeMap) {
        delete item.second;
        item.second = nullptr;
    }
}

ULL getCpuClockCycle()
{
    ULL t1 = athread_time_cycle();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ULL t2 = athread_time_cycle();
    return t2 - t1;
}