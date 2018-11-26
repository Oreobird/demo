#include <cstdint>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <jsoncpp/json/json.h>

#include <iomanip>

#include "evg_quality.h"
#include "evg_detector.h"

using namespace std;

class EvgJson
{
public:
    EvgJson();
    EvgJson(std::string json_path);
    int gen_exception_json(int face_num);
    int gen_face_json(landmark_t *mark, Quality *qof);
private:
    std::string m_json_path;
};
