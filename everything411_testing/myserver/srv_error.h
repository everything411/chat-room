#ifndef __SRV_ERROR
#define __SRV_ERROR
#define err(connindex, ERR)                                                                            \
    (strcpy(send_buffer, "{\n\"type\":\"error\",\n\"status\":\"error\",\n\"value\":\"" ERR "\"\n}\n"), \
     sendclient(connindex))
#define syntax_err() err(connindex, "ERRSyntax")
#define user_not_found_err() err(connindex, "ERRUserrNotFound")
#define wrong_pass_err() err(connindex, "ERRWrongPassword")
#define already_login_err() err(connindex, "ERRUserIsOnline")
#define already_login_cli_err() err(connindex, "ERRClientHasLogin")
#define already_reg_err() err(connindex, "ERRUserExists")
#define not_login_err() err(connindex, "ERRClientNotLogin")
#define not_online_err() err(connindex, "ERRUserNotOnline")
#define illeagal_char_err() err(connindex, "ERRIlleagalChar")
#define not_implemented_err() err(connindex, "ERRNotImplemented")
#endif
