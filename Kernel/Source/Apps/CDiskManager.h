#ifndef _CDiskManager_H
#define _CDiskManager_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CDiskManager {
public:
	bool Initialize(void);
	bool Run(void);
	bool Destroy(void);

private:
	void ClearWindow(void);

	ushort SelectDisk(ushort uChannel);
	ushort SelectController(void);
	void DisplayDiskDetail(ushort uChannel, ushort uDisk);

	void Alert(const char *sText);

	char *sCmd;
	int iMaxCmdSize;
	int iCmdSize;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
