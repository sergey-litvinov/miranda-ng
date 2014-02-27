#include "common.h"

INT_PTR CDropbox::ProtoGetCaps(WPARAM wParam, LPARAM)
{
	switch(wParam)
	{
	case PFLAGNUM_1:
		return PF1_IM | PF1_FILESEND;
	case PFLAGNUM_2:
		return PF2_ONLINE;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)MODULE " ID";
	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR)"uid";
	}

	return 0;
}

INT_PTR CDropbox::ProtoSendFile(WPARAM wParam, LPARAM lParam)
{
	if (!HasAccessToken())
		return ACKRESULT_FAILED;

	CCSDATA *pccsd = (CCSDATA*)lParam;

	FileTransfer *ftp = new FileTransfer();
	ftp->pfts.flags = PFTS_SENDING | PFTS_UTF;
	ftp->pfts.hContact = pccsd->hContact;
	ftp->hContact = (INSTANCE->hContactTransfer) ? INSTANCE->hContactTransfer : pccsd->hContact;
	INSTANCE->hContactTransfer = 0;

	char **files = (char**)pccsd->lParam;

	for (int i = 0; files[i]; i++)
	{
		if (PathIsDirectoryA(files[i]))
			ftp->totalFolders++;
		else
			ftp->pfts.totalFiles++;
	}

	ftp->pszFolders = new char*[ftp->totalFolders + 1];
	ftp->pszFolders[ftp->totalFolders] = NULL;

	ftp->pfts.pszFiles = new char*[ftp->pfts.totalFiles + 1];
	ftp->pfts.pszFiles[ftp->pfts.totalFiles] = NULL;

	for (int i = 0, j = 0, k = 0; files[i]; i++)
	{
		if (PathIsDirectoryA(files[i]))
		{
			if (!ftp->relativePathStart)
			{
				char *rootFolder = files[j];
				char *relativePath = strrchr(rootFolder, '\\') + 1;
				ftp->relativePathStart = relativePath - rootFolder;
			}

			ftp->pszFolders[j] = mir_strdup(&files[i][ftp->relativePathStart]);

			j++;
		}
		else
		{
			ftp->pfts.pszFiles[k] = mir_strdup(files[i]);

			FILE *file = fopen(files[i], "rb");
			if (file != NULL)
			{
				fseek(file, 0, SEEK_END);
				ftp->pfts.totalBytes += ftell(file);
				fseek(file, 0, SEEK_SET);
				fclose(file);
			}
			k++;
		}
	}

	ULONG fileId = InterlockedIncrement(&INSTANCE->hFileProcess);
	ftp->hProcess = (HANDLE)fileId;

	mir_forkthread(CDropbox::SendFileAsync, ftp);

	return fileId;
}

INT_PTR CDropbox::ProtoSendMessage(WPARAM wParam, LPARAM lParam)
{
	//CCSDATA *pccsd = (CCSDATA*)lParam;

	//char *message = (char*)pccsd->lParam;

	//if (message[0] && message[0] == '/')
	//{
	//	// parse commands
	//}

	return 0;
}

INT_PTR CDropbox::RequestApiAuthorization(WPARAM, LPARAM)
{
	mir_forkthread(CDropbox::RequestApiAuthorizationAsync, 0);

	return 0;
}

INT_PTR CDropbox::RevokeApiAuthorization(WPARAM, LPARAM)
{
	mir_forkthread(CDropbox::RevokeApiAuthorizationAsync, 0);

	return 0;
}

INT_PTR CDropbox::SendFilesToDropbox(WPARAM hContact, LPARAM)
{
	INSTANCE->hContactTransfer = hContact;

	HWND hwnd =  (HWND)CallService(MS_FILE_SENDFILE, INSTANCE->GetDefaultContact(), 0);

	dcftp[hwnd] = hContact;

	return 0;
}