package de.meilhaus.medriver;

public class MeIOStreamConfig{
	private int iChannel;
	private int iStreamConfig;
	private int iRef;
	private int iFlags;

	public MeIOStreamConfig(){
		this.iChannel = 0;
		this.iStreamConfig = 0;
		this.iRef = 0;
		this.iFlags = 0;
	}

	public MeIOStreamConfig(
			int iChannel,
			int iStreamConfig,
			int iRef,
			int iFlags){
		this.iChannel = iChannel;
		this.iStreamConfig = iStreamConfig;
		this.iRef = iRef;
		this.iFlags = iFlags;
	}

	public int getChannel(){
		return this.iChannel;
	}
	public int getStreamConfig(){
		return this.iStreamConfig;
	}
	public int getRef(){
		return this.iRef;
	}
	public int getFlags(){
		return this.iFlags;
	}

	public void setChannel(int iChannel){
		this.iChannel = iChannel;
	}
	public void setStreamConfig(int iStreamConfig){
		this.iStreamConfig = iStreamConfig;
	}
	public void setRef(int iRef){
		this.iRef = iRef;
	}
	public void setFlags(int iFlags){
		this.iFlags = iFlags;
	}
}
