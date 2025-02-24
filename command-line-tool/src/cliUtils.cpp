
#include "cliUtils.h"

DWORD CheckFile(std::wstring aFileName)
{
	DWORD lRetVal = ERROR_SUCCESS;
	PeSignatureVerifier::SignerInfoPtr lCertInfo = NULL;
	PeSignatureVerifier::TimeStampCertInfoPtr lTsCertInfo = NULL;

	/*std::wstring lSha256Wstr;
	lRetVal = PeSignatureVerifier::CalculateFileHash(
		aFileName,
		L"SHA256",
		lSha256Wstr);*/

	lRetVal = PeSignatureVerifier::CheckFileSignature(aFileName);

	/*std::wcout << L"File name: " << aFileName.c_str() << std::endl;
	if (lRetVal == ERROR_SUCCESS)
	{
		std::wcout << L"Verified: " << L"Signed" << std::endl;
	}
	else
	{
		std::wcout << L"Verified: " << L"Unsigned" << std::endl;
	}*/

	//std::wcout << L"SHA256: " << lSha256Wstr.c_str() << std::endl;

	/*lRetVal = PeSignatureVerifier::GetCertificateInfo(aFileName, lCertInfo);

	if (lRetVal != ERROR_SUCCESS)
	{
		return lRetVal;
	}*/

	//lCertInfo->PrintCertificateInfo();

	//lRetVal = PeSignatureVerifier::GetTimestampCertificateInfo(aFileName, lTsCertInfo);

	/*if (lRetVal != ERROR_SUCCESS)
	{
		return lRetVal;
	}*/

	//std::wcout << "Signing date: " << lTsCertInfo->GetDateAsWstr().c_str() << std::endl;

	return ERROR_SUCCESS;
}

bool isFileExists(std::wstring aFileName)
{
	auto lFileAttr = GetFileAttributes(aFileName.c_str());
	if (lFileAttr == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}

	return true;
}

bool isFileAFolder(std::wstring aFileName)
{
	auto lFileAttr = GetFileAttributes(aFileName.c_str());
	if (lFileAttr == FILE_ATTRIBUTE_DIRECTORY)
	{
		return false;
	}

	return false;
}
