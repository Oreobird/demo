#include <json-c/json.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "upgrade.h"

#define UG_ERR printf
#define UG_DBG printf

struct s_firm firm;

static char *get_time(void)
{
    static char date_str[20];
    time_t date;

    time(&date);
    strftime(date_str, sizeof(date_str), "%Y-%m-%d %H:%M:%S", localtime(&date));
    return date_str;
}


static int upgrade_info_query()
{
	return 0;
}

static int firm_url_get(char *firm_url)
{
	char url[256] = "http://192.168.10.205:4455/haha.bin";
	int ret = ERR;
	strncpy(firm_url, url, sizeof(url));
	return ret;
}


static int wget_secu_exec(char *firm_url) 
{
	pid_t pid;
	char *const argv[] = {"wget", "-qO", FIRM_PATH, firm_url, NULL};
	char *const envp[] = {NULL};
	int ret = ERR;
	
	if (access(FIRM_PATH, F_OK) != 0)
	{
		FILE *fp = fopen(FIRM_PATH, "w+");
		if (fp == NULL)
		{
			UG_ERR("fopen failed\n");
			return ret;
		}
		fclose(fp);
	}
	
	pid = fork();
	if (pid == -1) 
	{
		UG_ERR("fork error");
	}
	else if (pid == 0)
	{
		if (execve("/usr/bin/wget", argv, envp) == -1)
		{
			UG_ERR("Error executing wget");
			exit(GET_FIRM_ERR);
		}
	}
	else
	{
		int status;
		waitpid(pid, &status, 0);
		ret = WIFEXITED(status) ? WEXITSTATUS(status) : OK;
	}
	
	return ret;
}   

static int cmd_check_firm(unsigned char *cmd, int len,  void *ctx)
{
	unsigned char *firm_path = (unsigned char *)ctx;
	return snprintf(cmd, len, "/sbin/sysupgrade -T '%s' > /dev/null && echo 1 || echo 0", firm_path);
}

static int cmd_get_firm(unsigned char *cmd, int len,  void *ctx)
{
	unsigned char *firm_url = (unsigned char *)ctx;
	return snprintf(cmd, len, "wget -qO ~/upgrade.bin '%s' > /dev/null && echo 1 || echo 0", firm_url);
}

static int cmd_upgrade(unsigned char *cmd, int len, void *ctx)
{
	unsigned char *firm_path = (unsigned char *)ctx;
	return snprintf(cmd, len, "/sbin/sysupgrade '%s' > /dev/null && echo 1 || echo 0", firm_path);
}

static int shell_run(void *ctx, set_cmd_fn set_cmd)
{
	FILE *fp = NULL;
	unsigned char cmd[512] = {0};
	unsigned char buf[16] = {0};
	int ret = ERR;
	
	memset(cmd, 0, sizeof(cmd));
	set_cmd(cmd, sizeof(cmd), ctx);

	UG_DBG("cmd: %s", cmd);
	fp = popen(cmd, "r");
	if (!fp)
	{
		UG_ERR("run_shell: popen failed\n");
		return ret;
	}

	if (NULL == fgets(buf, sizeof(buf), fp))
	{
		UG_ERR("run_shell: fgets failed\n");
		pclose(fp);
		return ret;
	}

	pclose(fp);
	ret = atoi(buf) ? OK : ERR;
	
	return ret;
}

static int firm_check(unsigned char *firm_path)
{
	return shell_run(firm_path, cmd_check_firm);
}

static int firm_get(unsigned char *firm_url)
{
#if 0
	return shell_run(firm_url, cmd_get_firm);
#else
	return wget_secu_exec(firm_url);
#endif
}

static int upgrade(unsigned char *firm_path)
{
	return shell_run(firm_path, cmd_upgrade);
}


#define TEST
#ifdef TEST
int main(int argc, char **argv)
{
	char url[256] = {0};

	firm_url_get(url);
	
	if (firm_get(url) == OK)
	{
		if (firm_check(FIRM_PATH) == OK)
		{
			UG_DBG("check_firm OK\n");
		#if 0
			if (upgrade(FIRM_PATH) != OK)
				UG_ERR("upgrade failed\n");	
		#endif
		}
		else
		{
			UG_ERR("check_firm failed\n");
		}
	} 
	else 
	{
		UG_ERR("get_firm failed\n");
	}
	
    return 0;
}
#endif
