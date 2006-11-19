/*
    This file is part of OpenRRCP

    OpenRRCP is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    OpenRRCP is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenRRCP; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    ---

    You can send your updates, patches and suggestions on this software
    to it's original author, Andrew Chernyak (nording@yandex.ru)
    This would be appreciated, however not required.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "rrcp_packet.h"
#include "rrcp_io.h"
#include "rrcp_switches.h"
#include "../lib/libcli.h"
#include "rrcp_cli_cmd_show.h"
#include "rrcp_cli_cmd_config.h"

#define CLITEST_PORT		8000

int check_auth(char *username, char *password)
{
    if (!strcasecmp(username, "1") && !strcasecmp(password, "1"))
	return 1;
    return 0;
}

int check_enable(char *password)
{
    if (!strcasecmp(password, "1"))
        return 1;
    return 0;
}

void pc(struct cli_def *cli, char *string)
{
	printf("%s\n", string);
}

int main(int argc, char *argv[])
{
    struct cli_def *cli;
    int s, x;
    struct sockaddr_in servaddr;
    int on = 1;

    cli = cli_init();
    cli_set_banner(cli, "libcli test environment");
    cli_set_hostname(cli, "router");

    cmd_show_register_commands(cli);
    cmd_config_register_commands(cli);

//    cli_set_auth_callback(cli, check_auth);
//    cli_set_enable_callback(cli, check_enable);
/*
    // Test reading from a file
    {
	    FILE *fh;

	    if ((fh = fopen("clitest.txt", "r")))
	    {
		    // This sets a callback which just displays the cli_print() text to stdout
		    cli_print_callback(cli, pc);
		    cli_file(cli, fh, PRIVILEGE_UNPRIVILEGED, MODE_EXEC);
		    cli_print_callback(cli, NULL);
		    fclose(fh);
	    }
    }
*/
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
	perror("socket");
	return 1;
    }
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(CLITEST_PORT);
    if (bind(s, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
	perror("bind");
	return 1;
    }

    if (listen(s, 50) < 0)
    {
	perror("listen");
	return 1;
    }

    printf("Listening on port %d\n", CLITEST_PORT);
    while ((x = accept(s, NULL, 0)))
    {
	cli_loop(cli, x);
	close(x);
    }

    cli_done(cli);
    return 0;
}

