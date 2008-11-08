/* NickServ core functions
 *
 * (C) 2003-2008 Anope Team
 * Contact us at info@anope.org
 *
 * Please read COPYING and README for further details.
 *
 * Based on the original code of Epona by Lara.
 * Based on the original code of Services by Andy Church. 
 * 
 * $Id$
 *
 */
/*************************************************************************/

#include "module.h"

int do_status(User * u);
void myNickServHelp(User * u);

class NSStatus : public Module
{
 public:
	NSStatus(const std::string &modname, const std::string &creator) : Module(creator)
	{
		Command *c;

		moduleAddAuthor("Anope");
		moduleAddVersion("$Id$");
		moduleSetType(CORE);

		c = createCommand("STATUS", do_status, NULL, NICK_HELP_STATUS, -1, -1, -1, -1);
		moduleAddCommand(NICKSERV, c, MOD_UNIQUE);

		moduleSetNickHelp(myNickServHelp);
	}
};

/**
 * Add the help response to anopes /ns help output.
 * @param u The user who is requesting help
 **/
void myNickServHelp(User * u)
{
    notice_lang(s_NickServ, u, NICK_HELP_CMD_STATUS);
}

/**
 * The /ns status command.
 * @param u The user who issued the command
 * @param MOD_CONT to continue processing other modules, MOD_STOP to stop processing.
 **/
int do_status(User * u)
{
    User *u2;
    NickAlias *na = NULL;
    int i = 0;
    char *nick = strtok(NULL, " ");

    /* If no nickname is given, we assume that the user
     * is asking for himself */
    if (!nick)
        nick = u->nick;

    while (nick && (i++ < 16)) {
        if (!(u2 = finduser(nick)))     /* Nick is not online */
            notice_lang(s_NickServ, u, NICK_STATUS_0, nick);
        else if (nick_identified(u2))   /* Nick is identified */
            notice_lang(s_NickServ, u, NICK_STATUS_3, nick);
        else if (nick_recognized(u2))   /* Nick is recognised, but NOT identified */
            notice_lang(s_NickServ, u, NICK_STATUS_2, nick);
        else if ((na = findnick(nick)) == NULL) /* Nick is online, but NOT a registered */
            notice_lang(s_NickServ, u, NICK_STATUS_0, nick);
        else
            notice_lang(s_NickServ, u, NICK_STATUS_1, nick);

        /* Get the next nickname */
        nick = strtok(NULL, " ");
    }
    return MOD_CONT;
}

MODULE_INIT("ns_status", NSStatus)
