﻿#ifndef _STEAM_POOL_H_
#define _STEAM_POOL_H_

namespace SteamWebApi
{
	class PollApi : public BaseApi
	{
	public:
		enum POOL_TYPE
		{
			UNKNOWN = 0,
			MESSAGE = 1,
			TYPING = 2,
			STATE = 3,
			//POOL_TYPE_RELATIONSHIP = 4
		};

		class PoolItem : public Result
		{
			friend PollApi;

		private:
			std::string steamId;
			DWORD timestamp;
			POOL_TYPE type;
			bool send;

		public:
			PoolItem() : timestamp(0), type(POOL_TYPE::UNKNOWN) { }

			const char *GetSteamId() const { return steamId.c_str(); }
			const DWORD GetTimestamp() const { return timestamp; }
			POOL_TYPE GetType() const { return type; }
			bool IsSend() const { return send; }
		};

		class Typing : PoolItem { friend PollApi; };

		class Message : PoolItem
		{
			friend PollApi;

		private:
			std::wstring text;

		public:
			const wchar_t *GetText() const { return text.c_str(); }
		};

		class State : PoolItem
		{
			friend PollApi;

		private:
			int status;
			std::wstring nickname;

		public:
			int GetStatus() const { return status; }
			const wchar_t *GetNickname() const { return nickname.c_str(); }
		};

		class PollResult : public Result
		{
			friend PollApi;

		private:
			UINT32 messageId;
			std::vector<PoolItem*> items;

		public:
			PollResult() : messageId(0) { }

			UINT32 GetMessageId() const { return messageId; }
			int GetItemCount() const { return items.size(); }
			const PoolItem * operator[](int idx) const { return items.at(idx); }
		};

		static void PollStatus(HANDLE hConnection, const char *token, const char *sessionId, UINT32 messageId, PollResult *pollResult)
		{
			pollResult->success = false;

			CMStringA data;
			data.AppendFormat("access_token=%s", token);
			data.AppendFormat("&umqid=%s", sessionId);
			data.AppendFormat("&message=%i", messageId);
			//data.Append("&sectimeout=30");

			HttpRequest request(hConnection, REQUEST_POST, STEAM_API_URL "/ISteamWebUserPresenceOAuth/Poll/v0001");
			request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
			request.SetData(data.GetBuffer(), data.GetLength());
			request.timeout = 35000;
			
			mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
			if (!response || response->resultCode != HTTP_STATUS_OK)
				return;

			JSONNODE *root = json_parse(response->pData), *node, *child;
			node = json_get(root, "error");
			ptrW error(json_as_string(node));
			/*if (!lstrcmpi(error, L"Not Logged On"))
			{
			}
			else */
			/*if (!lstrcmpi(error, L"Timeout"))
			{
				pollResult->messageId = messageId;
				pollResult->success = true;
				return;
			}*/
			else if (lstrcmpi(error, L"OK"))
				return;

			node = json_get(root, "messagelast");
			pollResult->messageId = json_as_int(node);

			node = json_get(root, "messages");
			root = json_as_array(node);
			if (root != NULL)
			{
				for (int i = 0;; i++)
				{
					child = json_at(root, i);
					if (child == NULL)
						break;

					PoolItem *item = NULL;

					node = json_get(child, "type");
					ptrW type(json_as_string(node));
					if (!lstrcmpi(type, L"saytext") || !lstrcmpi(type, L"emote"))
					{
						Message *message = new Message();
						message->type = POOL_TYPE::MESSAGE;

						node = json_get(child, "text");
						if (node != NULL) message->text = json_as_string(node);

						item = message;
					}
					/*if (!lstrcmpi(type, L"my_saytext") || !lstrcmpi(type, L"my_emote"))
					{
						poll->type = POOL_TYPE::MESSAGE;
					}*/
					else if(!lstrcmpi(type, L"typing"))
					{
						item = new Typing();
						item->type = POOL_TYPE::TYPING;
					}
					else if (!lstrcmpi(type, L"personastate"))
					{
						State *state = new State();
						state->type = POOL_TYPE::STATE;

						node = json_get(child, "persona_state");
						if (node != NULL) state->status = json_as_int(node);

						node = json_get(child, "personaname");
						if (node != NULL) state->nickname = json_as_string(node);

						item = state;
					}
					/*else if (!lstrcmpi(type, L"personarelationship"))
					{
						type = (int)POOL_TYPE::RELATIONSHIP;
					}*/
					/*else if (!lstrcmpi(type, L"leftconversation"))
					{
					}*/
					else
					{
						int z = 0;
					}

					node = json_get(child, "steamid_from");
					item->steamId = ptrA(mir_u2a(json_as_string(node)));

					node = json_get(child, "utc_timestamp");
					item->timestamp = atol(ptrA(mir_u2a(json_as_string(node))));

					if (item != NULL)
						pollResult->items.push_back(item);
				}
			}

			pollResult->success = true;
		}
	};
}


#endif //_STEAM_POOL_H_