/* NickServ core functions
 *
 * (C) 2003-2011 Anope Team
 * Contact us at team@anope.org
 *
 * Please read COPYING and README for further details.
 *
 * Based on the original code of Epona by Lara.
 * Based on the original code of Services by Andy Church.
 */

/*************************************************************************/

#include "module.h"

static bool SendConfirmMail(User *u)
{
	int chars[] = {
		' ', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
		'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
		'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
		'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
		'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
	};
	int idx, min = 1, max = 62;
	Anope::string code;
	for (idx = 0; idx < 9; ++idx)
		code += chars[1 + static_cast<int>((static_cast<float>(max - min)) * getrandom16() / 65536.0) + min];
	u->Account()->Extend("ns_set_email_passcode", new ExtensibleItemRegular<Anope::string>(code));

	Anope::string subject = _("Email confirmation");
	Anope::string message = Anope::printf(_("Hi,\n"
	" \n"
	"You have requested to change your email address to %s.\n"
	"Please type \" %s%s confirm %s \" to confirm this change.\n"
	" \n"
	"If you don't know why this mail was sent to you, please ignore it silently.\n"
	" \n"
	"%s administrators."), u->Account()->email.c_str(), Config->UseStrictPrivMsgString.c_str(), Config->s_NickServ.c_str(), code.c_str(), Config->NetworkName.c_str());

	return Mail(u, u->Account(), NickServ, subject, message);
}

class CommandNSSetEmail : public Command
{
 public:
	CommandNSSetEmail(const Anope::string &spermission = "") : Command("EMAIL", 1, 2, spermission)
	{
		this->SetDesc(_("Associate an E-mail address with your nickname"));
	}

	CommandReturn Execute(CommandSource &source, const std::vector<Anope::string> &params)
	{
		User *u = source.u;
		NickAlias *na = findnick(params[0]);
		if (!na)
			throw CoreException("NULL na in CommandNSSetEmail");
		NickCore *nc = na->nc;

		Anope::string param = params.size() > 1 ? params[1] : "";

		if (param.empty() && Config->NSForceEmail)
		{
			source.Reply(_("You cannot unset the e-mail on this network."));
			return MOD_CONT;
		}
		else if (Config->NSSecureAdmins && u->Account() != nc && nc->IsServicesOper())
		{
			source.Reply(_(ACCESS_DENIED));
			return MOD_CONT;
		}
		else if (!param.empty() && !MailValidate(param))
		{
			source.Reply(_(MAIL_X_INVALID), param.c_str());
			return MOD_CONT;
		}

		if (!param.empty() && Config->NSConfirmEmailChanges && !u->IsServicesOper())
		{
			u->Account()->Extend("ns_set_email", new ExtensibleItemRegular<Anope::string>(param));
			Anope::string old = u->Account()->email;
			u->Account()->email = param;
			if (SendConfirmMail(u))
				source.Reply(_("A confirmation email has been sent to \002%s\002. Follow the instructions in it to change your email address."), param.c_str());
			u->Account()->email = old;
		}
		else
		{
			if (!param.empty())
			{
				nc->email = param;
				source.Reply(_("E-mail address for \002%s\002 changed to \002%s\002."), nc->display.c_str(), param.c_str());
			}
			else
			{
				nc->email.clear();
				source.Reply(_("E-mail address for \002%s\002 unset."), nc->display.c_str());
			}
		}

		return MOD_CONT;
	}

	bool OnHelp(CommandSource &source, const Anope::string &)
	{
		source.Reply(_("Syntax: \002SET EMAIL \037address\037\002\n"
				" \n"
				"Associates the given E-mail address with your nickname.\n"
				"This address will be displayed whenever someone requests\n"
				"information on the nickname with the \002INFO\002 command."));
		return true;
	}
};

class CommandNSSASetEmail : public CommandNSSetEmail
{
 public:
	CommandNSSASetEmail() : CommandNSSetEmail("nickserv/saset/email")
	{
	}

	bool OnHelp(CommandSource &source, const Anope::string &)
	{
		source.Reply(_("Syntax: \002SASET \037nickname\037 EMAIL \037address\037\002\n"
				" \n"
				"Associates the given E-mail address with the nickname."));
		return true;
	}
};

class NSSetEmail : public Module
{
	CommandNSSetEmail commandnssetemail;
	CommandNSSASetEmail commandnssasetemail;

 public:
	NSSetEmail(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator)
	{
		this->SetAuthor("Anope");
		this->SetType(CORE);

		ModuleManager::Attach(I_OnPreCommand, this);

		Command *c = FindCommand(NickServ, "SET");
		if (c)
			c->AddSubcommand(this, &commandnssetemail);

		c = FindCommand(NickServ, "SASET");
		if (c)
			c->AddSubcommand(this, &commandnssasetemail);
	}

	~NSSetEmail()
	{
		Command *c = FindCommand(NickServ, "SET");
		if (c)
			c->DelSubcommand(&commandnssetemail);

		c = FindCommand(NickServ, "SASET");
		if (c)
			c->DelSubcommand(&commandnssasetemail);
	}

	EventReturn OnPreCommand(CommandSource &source, Command *command, const std::vector<Anope::string> &params)
	{
		User *u = source.u;
		if (command->service == NickServ && command->name.equals_ci("CONFIRM") && !params.empty() && u->IsIdentified())
		{
			Anope::string new_email, passcode;
			if (u->Account()->GetExtRegular("ns_set_email", new_email) && u->Account()->GetExtRegular("ns_set_email_passcode", passcode))
			{
				if (params[0] == passcode)
				{
					u->Account()->email = new_email;
					Log(LOG_COMMAND, u, command) << "to confirm their email address change to " << u->Account()->email;
					source.Reply(_("Your email address has been changed to \002%s\002."), u->Account()->email.c_str());
					u->Account()->Shrink("ns_set_email");
					u->Account()->Shrink("ns_set_email_passcode");
					return EVENT_STOP;
				}
			}
		}

		return EVENT_CONTINUE;
	}
};

MODULE_INIT(NSSetEmail)
