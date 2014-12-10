#ifdef _MSC_VER
	//nop
#else
#include "common.h"

#include "FHCAPI.h"
#include "XLStringUtil.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#define FUSE_USE_VERSION 30
#include <fuse.h>

#define SLASH	'/'

static FHCAPI  g_API;


static void usage()
{
	printf("-家mount- HOMEFS (FHCFS)  with fusefs\n");
	printf("\n");
	printf("homefs ip@apikey directory [-o debug]\n");
	printf("\n");
	printf("mount.\n");
	printf(" 192.168.10.21のFHC鯖を /mnt にmountする。 デバッグモードでログを出力する.\n");
	printf(" homefs 192.168.10.21@webapi_XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX /mnt -o debug\n");
	printf("\n");
	printf("unmount.\n");
	printf(" umount /mnt\n");
	printf("\n");
	printf("実行には、 fusefs が必要です.\n");
	printf("\n");
}

static int fhcfs_getattr(const char *path
	, struct stat *stbuf)
{
	memset(stbuf, 0, sizeof(struct stat));
	if ( path[0]==SLASH && path[1]=='\0')
	{//トップディレクトリ
		const std::vector<Elec*>* elec = g_API.getElec();

		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2 + elec->size();
		return 0;
	}

	const char * splitSlash = strchr(path+1,SLASH);
	if (splitSlash == NULL)
	{// トップ直下には、家電名のディレクトリしか無い.
		const Elec* elec = g_API.findElec(path+1);
		if(!elec)
		{
			return -ENOENT;
		}

		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2 + elec->action.size();
		return 0;
	}

	//サブディレクトリには、家電の状態名とかがある.
	const std::string elecname = std::string(path+1,0,splitSlash - (path+1));
	const Elec* elec = g_API.findElec(elecname);
	if(!elec)
	{
		return -ENOENT;
	}

	if ( strcmp(splitSlash+1,".status") == 0 )
	{
		stbuf->st_mode = S_IFREG | 0744;
		stbuf->st_nlink = 1;
		stbuf->st_size = elec->status.size();
		return 0;
	}
	else if ( splitSlash+1 == elec->status)
	{//家電の状態ファイル. このステータスが有効な場合
		stbuf->st_mode = S_IFREG | 0744;
		stbuf->st_nlink = 1;
		stbuf->st_size = 1;	//"1"の長さ.
		return 0;
	}
	else
	{//家電の状態ファイル.
		stbuf->st_mode = S_IFREG | 0744;
		stbuf->st_nlink = 1;
		stbuf->st_size = 0; //""の長さ
		return 0;
	}
}

static int fhcfs_open(const char *path
	, struct fuse_file_info *fi)
{
	const char * splitSlash = strchr(path+1,SLASH);
	if (splitSlash == NULL)
	{//トップ直下には、家電名のディレクトリしか無い.
	 //ディレクトリは開けないのでエラーを返す
		return -ENOENT;
	}

	//サブディレクトリには、家電の状態名とかがある.
	const std::string elecname = std::string(path+1,0,splitSlash - (path+1));
	const Elec* elec = g_API.findElec(elecname);
	if(!elec)
	{
		return -ENOENT;
	}

	if ( strcmp(splitSlash+1,".status") == 0 )
	{
		return 0;
	}

	{
		for(auto ait = elec->action.begin(); ait!=elec->action.end() ; ait++)
		{
			if(splitSlash+1 == (*ait)->name)
			{
				return 0;
			}
		}
	}

	return -ENOENT;
}
static int fhcfs_read(const char *path
	,char *buf
	,size_t size
	,off_t offset
	, struct fuse_file_info *fi)
{
	const char * splitSlash = strchr(path+1,SLASH);
	if (splitSlash == NULL)
	{//トップ直下には、家電名のディレクトリしか無い.
	 //ディレクトリは開けないのでエラーを返す
		return -ENOENT;
	}

	//サブディレクトリには、家電の状態名とかがある.
	const std::string elecname = std::string(path+1,0,splitSlash - (path+1));
	const Elec* elec = g_API.findElec(elecname);
	if(!elec)
	{
		return -ENOENT;
	}

	std::string result;
	if ( strcmp(splitSlash+1,".status") == 0 )
	{
		result = elec->status;
	}
	else if ( splitSlash+1 == elec->status)
	{//家電の状態ファイル. このステータスが有効な場合
		result = "1";
	}
	else
	{//家電の状態ファイル.
		result = "";
	}
	
	if ( offset >= (off_t)result.size() )
	{//書き込みバッファがたりなさすぎ
		return 0;
	}
	if ( (offset + size) > (off_t)result.size() )
	{//ファイルサイズに、バッファを補正する.
		size = (result.size() - offset);
	}
	memcpy(buf, result.c_str() + offset, size);
	return size;
}

static int fhcfs_write(const char *path
	,const char *buf
	,size_t size
	,off_t offset
	, struct fuse_file_info *fi)
{
	const char * splitSlash = strchr(path+1,SLASH);
	if (splitSlash == NULL)
	{//トップ直下には、家電名のディレクトリしか無い.
	 //ディレクトリは開けないのでエラーを返す
		return -ENOENT;
	}
	if(offset!=0)
	{//ごめんね。まとめての読み書きしかサポートしないんだ.
		return -ENOENT;
	}

	//サブディレクトリには、家電の状態名とかがある.
	const std::string elecname = std::string(path+1,0,splitSlash - (path+1));
	const Elec* elec = g_API.findElec(elecname);
	if(!elec)
	{
		return -ENOENT;
	}

	if ( strcmp(splitSlash+1,".status") == 0 )
	{
		const std::string actionname = XLStringUtil::chop( std::string(buf,0,size) );

		//この状態名があるかどうか調べようか.
		if (! g_API.checkStatus(elecname,actionname) )
		{//そんな状態ないです。
			return -EIO;
		}

		g_API.fire(elecname,actionname);
	}
	else
	{//家電の状態ファイル.
		//ONにしたいんだよね・・・？
		bool isON = stringbool( std::string(buf,0,size) );
		if (!isON)
		{//ステータスファイル名でoffと言われてもどうしろと.
			return size;
		}

		//念のため、パスが有効な状態名かを確認する.
		if (! g_API.checkStatus(elecname,splitSlash+1) )
		{//そんな状態ないです。
			return -EIO;
		}

		g_API.fire(elecname,splitSlash+1);
	}
	g_API.update();

	return size;
}



static int fhcfs_setattr(const char *path
	,const char *name
	,const char *value
	,size_t size
	,int flags)
{
	struct stat stbuf;
	return fhcfs_getattr(path,&stbuf);
}
static int fhcfs_chmod(const char *path, mode_t mode)
{
	struct stat stbuf;
	return fhcfs_getattr(path,&stbuf);
}

static int fhcfs_chown(const char *path, uid_t uid, gid_t gid)
{
	struct stat stbuf;
	return fhcfs_getattr(path,&stbuf);
}

static int fhcfs_truncate(const char *path, off_t size)
{
	struct stat stbuf;
	return fhcfs_getattr(path,&stbuf);
}

static int fhcfs_utime(const char *path, utimbuf*)
{
	struct stat stbuf;
	return fhcfs_getattr(path,&stbuf);
}

static int fhcfs_readdir(const char *path
	, void *buf
	, fuse_fill_dir_t filler
	, off_t offset
	, struct fuse_file_info *fi)
{
	if ( path[0] != SLASH )
	{//謎
		return -ENOENT;
	}
	g_API.update();

	if ( path[1] == '\0' )
	{// トップディレクトリ
		filler(buf, ".", NULL, 0);
		filler(buf, "..", NULL, 0);

		const std::vector<Elec*>* elec = g_API.getElec();
		for(auto eit = elec->begin(); eit!=elec->end() ; eit++)
		{
			filler(buf, (*eit)->name.c_str() , NULL, 0);
		}
		return 0;
	}

	const char * splitSlash = strchr(path+1,SLASH);
	if (splitSlash == NULL)
	{
		splitSlash = path+strlen(path);
	}

	{
		const std::string elecname = std::string(path+1,0,splitSlash - (path+1));
		const Elec* elec = g_API.findElec(elecname);
		if (!elec)
		{
			return -ENOENT;
		}

		filler(buf, ".", NULL, 0);
		filler(buf, "..", NULL, 0);
		filler(buf, ".status", NULL, 0); //現在の状態 

		for(auto ait = elec->action.begin(); ait!=elec->action.end() ; ait++)
		{
			filler(buf, (*ait)->name.c_str() , NULL, 0);
		}
	}
	return 0;
}


struct fuse_operations fhcfs_oper ={NULL};

int main(int argc, char *argv[])
{
	fhcfs_oper.getattr	= fhcfs_getattr;
	fhcfs_oper.readdir	= fhcfs_readdir;
	fhcfs_oper.open		= fhcfs_open;
	fhcfs_oper.read		= fhcfs_read;
	fhcfs_oper.write	= fhcfs_write;
	fhcfs_oper.truncate = fhcfs_truncate;
	fhcfs_oper.chmod = fhcfs_chmod;
	fhcfs_oper.chown = fhcfs_chown;
	fhcfs_oper.utime = fhcfs_utime;

	if(argc<=2)
	{
		usage();
		return 0;
	}
	
	char *p = strstr(argv[1],"@");
	if(!p)
	{
		usage();
		return 0;
	}
	std::string mountpoint = std::string(argv[1],0,(int)(p-argv[1]));
	std::string apikey = p+1;
	
	//パスワード(apikey)をps aux で見られないようにマスクする.
	memset(p,'#',apikey.size());

	//1番目のmount元はfusefsとしては間違っている構文なので、スキップする.
	int argcc=0;
	char *argvv[argc];
	for(int i = 0 ; i < argc ; i ++ )
	{
		if (i==1 )
		{
			continue;
		}

		argvv[argcc]=argv[i];
		argcc++;
	}

	//サーバ名とapikeyをセット
	if(!g_API.setInit(mountpoint,apikey))
	{
		fprintf(stderr,"g_API.parseLine error!\n");
		return -1;
	}

	//FHCと通信して、ファイルシステムのアップデート
	if(!g_API.update())
	{
		fprintf(stderr,"g_API.update error!\n");
		return -1;
	}

	//fusefs先生あとはよろしくおなしゃす
	return fuse_main(argcc, argvv, &fhcfs_oper,NULL);
}

#endif
