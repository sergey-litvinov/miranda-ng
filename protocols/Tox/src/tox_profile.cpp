#include "common.h"

std::tstring CToxProto::GetToxProfilePath()
{
	return GetToxProfilePath(m_tszUserName);
}

std::tstring CToxProto::GetToxProfilePath(const TCHAR *accountName)
{
	std::tstring profilePath;
	TCHAR defaultPath[MAX_PATH];
	mir_sntprintf(defaultPath, MAX_PATH, _T("%s\\%s.tox"), VARST(_T("%miranda_userdata%")), accountName);
	profilePath = defaultPath;

	return profilePath;
}

bool CToxProto::LoadToxProfile()
{
	DWORD size = GetFileSize(hProfile, NULL);
	if (size == 0)
	{
		debugLogA("CToxProto::LoadToxData: tox profile is empty");
		return false;
	}

	DWORD read = 0;
	uint8_t *data = (uint8_t*)mir_calloc(size);
	if (!ReadFile(hProfile, data, size, &read, NULL) || size != read)
	{
		debugLogA("CToxProto::LoadToxData: could not read tox profile");
		mir_free(data);
		return false;
	}

	if (tox_is_data_encrypted(data))
	{
		ptrA password(mir_utf8encodeW(ptrT(getTStringA("Password"))));
		if (password == NULL || strlen(password) == 0)
		{
			INT_PTR result = DialogBoxParam(
				g_hInstance,
				MAKEINTRESOURCE(IDD_PASSWORD),
				NULL,
				ToxProfilePasswordProc,
				(LPARAM)this);

			switch (result)
			{
			default: return false;
			}
		}

		if (tox_encrypted_load(tox, data, size, (uint8_t*)(char*)password, strlen(password)) == TOX_ERROR)
		{
			debugLogA("CToxProto::LoadToxData: could not decrypt tox profile");
			mir_free(data);
			return false;
		}
	}
	else
	{
		if (tox_load(tox, data, size) == TOX_ERROR)
		{
			debugLogA("CToxProto::LoadToxData: could not load tox profile");
			mir_free(data);
			return false;
		}
	}

	mir_free(data);

	return true;
}

void CToxProto::SaveToxProfile()
{
	ptrA password(mir_utf8encodeW(ptrT(getTStringA("Password"))));
	bool needToEncrypt = password && strlen(password);
	DWORD size = needToEncrypt
		? tox_encrypted_size(tox)
		: tox_size(tox);

	uint8_t *data = (uint8_t*)mir_calloc(size);
	if (needToEncrypt)
	{
		if (tox_encrypted_save(tox, data, (uint8_t*)(char*)password, strlen(password)) == TOX_ERROR)
		{
			debugLogA("CToxProto::LoadToxData: could not encrypt tox profile");
			mir_free(data);
			return;
		}
	}
	else
	{
		tox_save(tox, data);
	}

	SetFilePointer(hProfile, 0, NULL, FILE_BEGIN);
	SetEndOfFile(hProfile);
	DWORD written = 0;
	if (!WriteFile(hProfile, data, size, &written, NULL) || size != written)
	{
		debugLogA("CToxProto::LoadToxData: could not write tox profile");
	}

	mir_free(data);
}

INT_PTR CToxProto::ToxProfileImportProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TCHAR *accountName = (TCHAR*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	TCHAR *profilePath = (TCHAR*)GetWindowLongPtr(hwnd, DWLP_USER);

	switch (uMsg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		{
			accountName = (TCHAR*)lParam;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);

			profilePath = (TCHAR*)mir_calloc(sizeof(TCHAR)*MAX_PATH);
			SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)profilePath);
		}
		return TRUE;

	case WM_DESTROY:
		mir_free(profilePath);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_PROFILE_PATH:
			if ((HWND)lParam == GetFocus())
			{
				if (HIWORD(wParam) != EN_CHANGE) return 0;
				EnableWindow(GetDlgItem(hwnd, IDOK), TRUE);
			}
			break;

		case IDC_BROWSE_PROFILE:
		{
			TCHAR filter[MAX_PATH] = { 0 };
			mir_sntprintf(filter, MAX_PATH, _T("%s\0*.*"), TranslateT("All files (*.*)"));

			OPENFILENAME ofn = { sizeof(ofn) };
			ofn.hwndOwner = hwnd;
			ofn.lpstrFilter = filter;
			ofn.nFilterIndex = 1;
			ofn.lpstrFile = profilePath;
			ofn.lpstrTitle = TranslateT("Select tox profile");
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER;

			if (GetOpenFileName(&ofn) && profilePath)
			{
				EnableWindow(GetDlgItem(hwnd, IDOK), TRUE);
				SetDlgItemText(hwnd, IDC_PROFILE_PATH, profilePath);
			}
		}
			break;

		case IDOK:
		{
			std::tstring defaultProfilePath = GetToxProfilePath(accountName);
			if (profilePath && _tcslen(profilePath))
			{
				if (_tcsicmp(profilePath, defaultProfilePath.c_str()) != 0)
				{
					CopyFile(profilePath, defaultProfilePath.c_str(), FALSE);
				}
			}
			else
			{
				fclose(_wfopen(defaultProfilePath.c_str(), _T("w")));
			}
			EndDialog(hwnd, 1);
		}
			break;

		case IDCANCEL:
			EndDialog(hwnd, 0);
			break;
		}
		break;
	}

	return FALSE;
}

INT_PTR CToxProto::ToxProfilePasswordProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CToxProto *proto = (CToxProto*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (uMsg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		{
			proto = (CToxProto*)lParam;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hwnd, 1);
			break;

		case IDCANCEL:
			EndDialog(hwnd, 0);
			break;
		}
		break;
	}

	return FALSE;
}