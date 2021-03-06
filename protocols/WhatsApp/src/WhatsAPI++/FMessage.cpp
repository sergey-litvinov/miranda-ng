/*
 * FMessage.cpp
 *
 *  Created on: 02/07/2012
 *      Author: Antonio
 */

#include <ctime>
#include <stdlib.h>
#include <algorithm>
#include "FMessage.h"
#include "utilities.h"

//SDL_mutex* FMessage::generating_lock = SDL_CreateMutex();
IMutex* FMessage::generating_lock = NULL;
int FMessage::generating_id = 0;
std::string FMessage::generating_header = Utilities::intToStr(static_cast<int> (time(NULL))).append("-");


FMessage::FMessage() {
	this->key = NULL;
	this->timestamp = 0;
	this->media_wa_type = 0;
	this->longitude = 0;
	this->latitude = 0;
	this->media_duration_seconds = 0;
	this->media_size = 0;
	this->media_name = "";
	this->media_url = "";
	this->data = "";
}

FMessage::FMessage(const std::string& remote_jid, bool from_me, const std::string& data) {
	Key* local_key;
	FMessage::generating_lock->lock();
	FMessage::generating_id++;
	local_key = new Key(remote_jid, from_me, generating_header + Utilities::intToStr(generating_id));
	this->key = local_key;
	FMessage::generating_lock->unlock();
	this->data = data;
	this->timestamp = time(NULL);
	this->media_wa_type = 0;
	this->longitude = 0;
	this->latitude = 0;
	this->media_duration_seconds = 0;
	this->media_size = 0;
	this->media_name = "";
	this->media_url = "";
}

std::string FMessage::nextKeyIdNumber() {
	int id = 0;
	FMessage::generating_lock->lock();
	id = (FMessage::generating_id++);
	FMessage::generating_lock->unlock();
	return generating_header + (Utilities::intToStr(id));
}

FMessage::FMessage(Key* key) {
	this->key = key;
	this->timestamp = 0;
	this->media_wa_type = 0;
	this->longitude = 0;
	this->latitude = 0;
	this->media_duration_seconds = 0;
	this->media_size = 0;
	this->media_name = "";
	this->media_url = "";
}

std::string FMessage::getMessage_WA_Type_StrValue(unsigned char type) {
	switch (type) {
	case FMessage::WA_TYPE_UNDEFINED:
		return "";
	case FMessage::WA_TYPE_SYSTEM:
		return "system";
	case FMessage::WA_TYPE_AUDIO:
		return "audio";
	case FMessage::WA_TYPE_CONTACT:
		return "vcard";
	case FMessage::WA_TYPE_IMAGE:
		return "image";
	case FMessage::WA_TYPE_LOCATION:
		return "location";
	case FMessage::WA_TYPE_VIDEO:
		return "video";
	}

	return "";
}

FMessage::~FMessage() {
	if (this->key != NULL)
		delete key;
}

Key::Key(const std::string& remote_jid, bool from_me, const std::string& id) {
	this->remote_jid = remote_jid;
	this->from_me = from_me;
	this->id = id;
}

std::string Key::toString() {
	return "Key[id=" + id + ", from_me=" + (from_me ? "true":"false") + ", remote_jid=" + remote_jid + "]";
}


unsigned char FMessage::getMessage_WA_Type(const std::string& type) {
	if (type.empty())
		return WA_TYPE_UNDEFINED;
	
	std::string typeLower = type;
	std::transform(typeLower.begin(), typeLower.end(), typeLower.begin(), ::tolower);
	if (typeLower.compare("system") == 0)
		return WA_TYPE_SYSTEM;
	if (typeLower.compare("image") == 0)
		return WA_TYPE_IMAGE;
	if (typeLower.compare("audio") == 0)
		return WA_TYPE_AUDIO;
	if (typeLower.compare("video") == 0)
		return WA_TYPE_VIDEO;
	if (typeLower.compare("vcard") == 0)
		return WA_TYPE_CONTACT;
	if (typeLower.compare("location") == 0)
		return WA_TYPE_LOCATION;

	return WA_TYPE_UNDEFINED;
}
