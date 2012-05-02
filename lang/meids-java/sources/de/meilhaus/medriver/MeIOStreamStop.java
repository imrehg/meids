package de.meilhaus.medriver;

public class MeIOStreamStop{
	private int iDevice;
	private int iSubdevice;
	private int iStopMode;
	private int iFlags;
	private int iErrno;

	public MeIOStreamStop(){
		this.iDevice = 0;
		this.iSubdevice = 0;
		this.iStopMode = 0;
		this.iFlags = 0;
		this.iErrno = 0;
	}

	public MeIOStreamStop(
			int iDevice,
			int iSubdevice,
			int iStopMode,
			int iFlags){
		this.iDevice = iDevice;
		this.iSubdevice = iSubdevice;
		this.iStopMode = iStopMode;
		this.iFlags = iFlags;
		this.iErrno = 0;
	}

	public int getDevice(){
		return this.iDevice;
	}
	public int getSubdevice(){
		return this.iSubdevice;
	}
	public int getStopMode(){
		return this.iStopMode;
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
	public void setStopMode(int iStopMode){
		this.iStopMode = iStopMode;
	}
	public void setFlags(int iFlags){
		this.iFlags = iFlags;
	}
	public void setErrno(int iErrno){
		this.iErrno = iErrno;
	}
}
