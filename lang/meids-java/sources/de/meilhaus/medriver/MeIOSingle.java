package de.meilhaus.medriver;

public class MeIOSingle{
	private int iDevice;
	private int iSubdevice;
	private int iChannel;
	private int iDir;
	private int iValue;
	private int iTimeOut;
	private int iFlags;
	private int iErrno;

	public MeIOSingle(){
		iDevice = 0;
		iSubdevice = 0;
		iChannel = 0;
		iDir = 0;
		iValue = 0;
		iTimeOut = 0;
		iFlags = 0;
		iErrno = 0;
	}

	public MeIOSingle(
			int iDevice,
			int iSubdevice,
			int iChannel,
			int iDir,
			int iValue,
			int iTimeOut,
			int iFlags){
		this.iDevice = iDevice;
		this.iSubdevice = iSubdevice;
		this.iChannel = iChannel;
		this.iDir = iDir;
		this.iValue = iValue;
		this.iTimeOut = iTimeOut;
		this.iFlags = iFlags;
		this.iErrno = 0;
	}

	public int getDevice(){
		return this.iDevice;
	}
	public int getSubdevice(){
		return this.iSubdevice;
	}
	public int getChannel(){
		return this.iChannel;
	}
	public int getDir(){
		return this.iDir;
	}
	public int getValue(){
		return this.iValue;
	}
	public int getTimeOut(){
		return this.iTimeOut;
	}
	public int getFlags(){
		return this.iFlags;
	}
	public int getErrno(){
		return this.iErrno;
	}

	public void setDevice(int iDevice){
		this.iDevice = iDevice;
	}
	public void setSubdevice(int iSubdevice){
		this.iSubdevice = iSubdevice;
	}
	public void setChannel(int iChannel){
		this.iChannel = iChannel;
	}
	public void setDir(int iDir){
		this.iDir = iDir;
	}
	public void setValue(int iValue){
		this.iValue = iValue;
	}
	public void setTimeOut(int iTimeOut){
		this.iTimeOut = iTimeOut;
	}
	public void setFlags(int iFlags){
		this.iFlags = iFlags;
	}
	public void setErrno(int iErrno){
		this.iErrno = iErrno;
	}
}
