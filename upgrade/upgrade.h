#ifndef __UPGRADE_MGMT_H__
#define __UPGRADE_MGMT_H__

#define UPGRADE "/sbin/sysupgrade"
#define FIRM_PATH "/tmp/upgrade.bin"

#define OK 0
#define ERR -1
#define CHECK_FIRM_ERR 1
#define GET_FIRM_ERR 2
#define UPGRADE_ERR 3

typedef int (*set_cmd_fn) (unsigned char *cmd, int len, void *ctx);

struct s_firm 
{
	unsigned char version[64];
	unsigned char url[256];
	unsigned char save_path[256];
	
	int (*get_firm_url)(void);
	int (*check_version)(void);
	int (*check_firm)(void);
	int (*get_firm)(void);
	int (*upgrade)(void);
};

#endif
