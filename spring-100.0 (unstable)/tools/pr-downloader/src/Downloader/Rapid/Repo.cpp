/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "Repo.h"
#include "FileSystem/FileSystem.h"
#include "Downloader/IDownloader.h"
#include "RapidDownloader.h"
#include "Sdp.h"
#include "Util.h"
#include "Logger.h"

#include <zlib.h>
#include <stdio.h>


CRepo::CRepo(const std::string& repourl, CRapidDownloader* rapid):
	repourl(repourl),
	rapid(rapid)
{
}

CRepo::~CRepo()
{
}

bool CRepo::getDownload(IDownload& dl)
{
	std::string tmp;
	urlToPath(repourl,tmp);
	LOG_DEBUG("%s",tmp.c_str());
	this->tmpFile = fileSystem->getSpringDir() + PATH_DELIMITER + "rapid" + PATH_DELIMITER +tmp + PATH_DELIMITER +"versions.gz";
	fileSystem->createSubdirs(tmpFile);
	//first try already downloaded file, as repo master file rarely changes
	if ((fileSystem->fileExists(tmpFile)) && fileSystem->isOlder(tmpFile,REPO_RECHECK_TIME))
		return false;
	fileSystem->createSubdirs(tmpFile);
	dl = IDownload(tmpFile);
	dl.addMirror(repourl + "/versions.gz");
	return true;
}

bool CRepo::parse()
{
	LOG_DEBUG("%s",tmpFile.c_str());
	FILE* f = fileSystem->propen(tmpFile, "rb");
	gzFile fp = gzdopen(fileno(f), "rb");
	if (fp==Z_NULL) {
		fclose(f);
		LOG_ERROR("Could not open %s",tmpFile.c_str());
		return false;
	}

	char buf[IO_BUF_SIZE];
	sdps.clear();
	while ((gzgets(fp, buf, sizeof(buf)))!=Z_NULL) {
		for (unsigned int i=0; i<sizeof(buf); i++) {
			if (buf[i]=='\n') {
				buf[i]=0;
				break;
			}
		}

		std::string tmp=buf;
		std::string shortname;
		getStrByIdx(tmp,shortname, ',',0);
		std::string md5;
		getStrByIdx(tmp, md5, ',',1);
		std::string depends;
		getStrByIdx(tmp,depends, ',',2);
		std::string name;
		getStrByIdx(tmp,name, ',',3);
		if (shortname.size()>0) { //create new repo from url
			CSdp sdptmp=CSdp(shortname, md5, name, depends, repourl);
			rapid->addRemoteDsp(sdptmp);
		}
	}
	int errnum=Z_OK;
	const char* errstr = gzerror(fp, &errnum);
	switch(errnum) {
	case Z_OK:
	case Z_STREAM_END:
		break;
	default:
		LOG_ERROR("%d %s\n", errnum, errstr);
	}
	gzclose(fp);
	fclose(f);
	return true;
}
