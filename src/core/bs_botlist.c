/* BotServ core functions
 *
 * (C) 2003-2010 Anope Team
 * Contact us at team@anope.org
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

class CommandBSBotList : public Command
{
 public:
	CommandBSBotList() : Command("BOTLIST", 0, 0)
	{
	}

	CommandReturn Execute(User *u, const std::vector<ci::string> &params)
	{
		int i, count = 0;
		BotInfo *bi;

		if (!nbots) {
			notice_lang(Config.s_BotServ, u, BOT_BOTLIST_EMPTY);
			return MOD_CONT;
		}

		for (i = 0; i < 256; i++) {
			for (bi = botlists[i]; bi; bi = bi->next) {
				if (!(bi->HasFlag(BI_PRIVATE))) {
					if (!count)
						notice_lang(Config.s_BotServ, u, BOT_BOTLIST_HEADER);
					count++;
					u->SendMessage(Config.s_BotServ, "   %-15s  (%s@%s)", bi->nick.c_str(), bi->user.c_str(), bi->host.c_str());
				}
			}
		}

		if (u->Account()->HasCommand("botserv/botlist") && count < nbots) {
			notice_lang(Config.s_BotServ, u, BOT_BOTLIST_PRIVATE_HEADER);

			for (i = 0; i < 256; i++) {
				for (bi = botlists[i]; bi; bi = bi->next) {
					if (bi->HasFlag(BI_PRIVATE)) {
						u->SendMessage(Config.s_BotServ, "   %-15s  (%s@%s)", bi->nick.c_str(), bi->user.c_str(), bi->host.c_str());
						count++;
					}
				}
			}
		}

		if (!count)
			notice_lang(Config.s_BotServ, u, BOT_BOTLIST_EMPTY);
		else
			notice_lang(Config.s_BotServ, u, BOT_BOTLIST_FOOTER, count);
		return MOD_CONT;
	}

	bool OnHelp(User *u, const ci::string &subcommand)
	{
		notice_help(Config.s_BotServ, u, BOT_HELP_BOTLIST);
		return true;
	}
};

class BSBotList : public Module
{
 public:
	BSBotList(const std::string &modname, const std::string &creator) : Module(modname, creator)
	{
		this->SetAuthor("Anope");
		this->SetVersion("$Id$");
		this->SetType(CORE);
		this->AddCommand(BOTSERV, new CommandBSBotList());

		ModuleManager::Attach(I_OnBotServHelp, this);
	}
	void OnBotServHelp(User *u)
	{
		notice_lang(Config.s_BotServ, u, BOT_HELP_CMD_BOTLIST);
	}
};

MODULE_INIT(BSBotList)