﻿#ifndef _STEAM_LOGIN_H_
#define _STEAM_LOGIN_H_

namespace SteamWebApi
{
	class LoginApi : public BaseApi
	{
	public:
		class LoginResult : public Result
		{
			friend LoginApi;

		private:
			std::string steamid;
			std::string umqid;
			UINT32 messageId;

		public:

			const char *GetSteamId() { return steamid.c_str(); }
			const char *GetSessionId() { return umqid.c_str(); }
			UINT32 GetMessageId() { return messageId; }
		};
		
		static void Logon(HANDLE hConnection, const char *token, LoginResult *loginResult)
		{
			loginResult->success = false;

			CMStringA data;
			data.AppendFormat("access_token=%s", token);
			data.Append("&ui_mode=web");

			HttpRequest request(hConnection, REQUEST_POST, STEAM_API_URL "/ISteamWebUserPresenceOAuth/Logon/v0001");
			request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
			request.SetData(data.GetBuffer(), data.GetLength());
			
			mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
			if (!response || response->resultCode != HTTP_STATUS_OK)
				return;
			
			JSONNODE *root = json_parse(response->pData), *node;

			node = json_get(root, "error");
			ptrW error(json_as_string(node));
			if (lstrcmpi(error, L"OK"))
				return;

			node = json_get(root, "steamid");
			loginResult->steamid = ptrA(mir_u2a(json_as_string(node)));

			node = json_get(root, "umqid");
			loginResult->umqid = ptrA(mir_u2a(json_as_string(node)));

			node = json_get(root, "message");
			loginResult->messageId = json_as_int(node);

			loginResult->success = true;
		}

		static void Relogon(HANDLE hConnection, const char *token, LoginResult *loginResult)
		{
		}

		static void Logoff(HANDLE hConnection, const char *token, const char *sessionId)
		{
			CMStringA data;
			data.AppendFormat("access_token=%s", token);
			data.AppendFormat("&umqid=%s", sessionId);

			HttpRequest request(hConnection, REQUEST_POST, STEAM_API_URL "/ISteamWebUserPresenceOAuth/Logoff/v0001");
			request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
			request.SetData(data.GetBuffer(), data.GetLength());
			
			mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
		}
	};
}

#endif //_STEAM_LOGIN_H_