#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <iomanip>

#include "boost/program_options.hpp"
#include "boost/filesystem.hpp"
#include <boost/interprocess/ipc/message_queue.hpp>
#include<boost/date_time.hpp>

#include "evg_common.h"
#include <jsoncpp/json/json.h>
#include "evg_msg.h"

using namespace std;
using namespace boost::interprocess;
using namespace boost::posix_time;

std::string json_path;
std::string img_path;
std::string result_img_path;

void usage(char *bin_name)
{
    printf("usage: %s -i image_file -j json_file -o result.jpg -t type\n", bin_name);
   	printf("	-i: image file path to detect.\n");
   	printf("	-j: output json file path.\n");
   	printf("	-o: output detect result image file path.\n");
   	printf("	-t: landmark detect type, int [0, 2]\n");
    printf("eg: %s -i ./001.jpg -j ./quality.json -o ./result.jpg -t 0\n", bin_name);
}

int main(int argc, char** argv)
{
	char prog_path[256] = {0};
	char default_img[256] = {0};
	char default_json[256] = {0};
	char default_result[256] = {0};
	char ch;
	int mark_type = 0;

	char *p = strrchr(argv[0], '/');
	strncpy(prog_path, argv[0], p - argv[0]);

	while ((ch = getopt(argc, argv, "hj:i:o:t:")) != EOF)
	{
		switch (ch)
		{
			case 'j':
				json_path = optarg;
				break;
			case 'i':
				img_path = optarg;
				break;
			case 'o':
				result_img_path = optarg;
				break;
			case 't':
				mark_type = atoi(optarg);
				break;
			case 'h':
				usage(argv[0]);
				return 0;
			default:
				break;
		}
	}

	if (img_path.empty())
	{
		snprintf(default_img, sizeof(default_img), "%s/sample.jpg", prog_path);
		img_path = default_img;
	}
	if (json_path.empty())
	{
		snprintf(default_json, sizeof(default_json), "%s/quality.json", prog_path);
		json_path = default_json;
	}
	if (result_img_path.empty())
	{
		snprintf(default_result, sizeof(default_result), "%s/result.jpg", prog_path);
		result_img_path = default_result;
	}

	message_queue srv_mq(open_only, "aivideo_srv_queue");
	evg_msg_t msg;

	Json::FastWriter fw;
	Json::Value root;
	root["img_path"] = img_path;
	root["json_path"] = json_path;
	root["result_img_path"] = result_img_path;
	std::string json_str = fw.write(root);

	memset(&msg, 0, sizeof(msg));
	snprintf(msg.payload, sizeof(msg.payload), "%s", json_str.c_str());
	msg.len = json_str.length();

	srv_mq.send(&msg, sizeof(msg), 0);

	message_queue cli_mq(open_only, "aivideo_cli_queue");
	message_queue::size_type recv_size;
	unsigned int priority;

	memset(&msg, 0, sizeof(msg));
	ptime t = microsec_clock::universal_time() + milliseconds(5000);
	cli_mq.timed_receive(&msg, sizeof(msg), recv_size, priority, t);
	fa_dbg("msg:%s\n", msg.payload);

	return 0;
}
