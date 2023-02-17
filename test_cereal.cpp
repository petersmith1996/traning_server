#include <string>
#include "Cereal/types/string.hpp"
#include "Cereal/types/vector.hpp"
#include <iostream>
#include <fstream>      // ofstream, ifstream
#include <algorithm>    // for_each
#include "Cereal/archives/binary.hpp"

using namespace std;

struct DataStruct
{
  int nA;
  int nB;
  double dC;
  vector<string> vString;

  template <class Archive>
  void serialize(Archive& archive)
  {
    archive(nA, nB, dC, vString);
  }
};


void DataPrint(DataStruct data)
{
    printf("data1 : nA(%d), nB(%d), dC(%f), vString(", data.nA, data.nB, data.dC);
    for_each(data.vString.begin(), data.vString.end(), [](string& str) {
        printf("%s ", str.c_str());
        });
    printf(")\n");
}


int main(void)
{
    // 데이터 출력
    ofstream out("Test.dat", ios::binary);
    {
        cereal::BinaryOutputArchive oArchives(out);
 
        DataStruct data1;
        {
            data1.nA = 1;
            data1.nB = 2;
            data1.dC = 3.3;
            data1.vString.push_back("한글입니다.");
            data1.vString.push_back("English");
            data1.vString.push_back("Data 저장 확인");
        }
        DataStruct data2;
        {
            data2.nA = 10;
            data2.nB = 20;
            data2.dC = 30.3;
            data2.vString.push_back("옜다.");
            data2.vString.push_back("Take it.");
            data2.vString.push_back("Remix 뤼믹스");
        }
 
        oArchives(data2);
        printf("%s\n", (char*)out);
        //oArchives(data1, data2);
        out.close();
    }
 
    // 데이터 입력
    ifstream in("Test.dat", ios::binary);
    {
        cereal::BinaryInputArchive iArchives(in);
 
        DataStruct data1;
        DataStruct data2;
        //iArchives(data1, data2);
        iArchives(data2);
 
        //DataPrint(data1);
        DataPrint(data2);
 
        in.close();
    }
 
    return 0;
}
