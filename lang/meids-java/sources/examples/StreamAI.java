package examples;

import de.meilhaus.medriver.*;

public class StreamAI {
	public static void main(String[] args){
		MeDriver drv = new MeDriver();
		int[] acqArgs = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		int[] scanArgs = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		int[] convArgs = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

		try{
			drv.meOpen(0);

			int nd = drv.meQueryNumberDevices();
			if(nd > 0){
				System.out.println(nd + " devices detected by driver system.");
			}
			else{
				System.out.println("No devices detected by driver system.");
				return;
			}
			System.out.println("Search for first streaming analog input device.");

			int ns = drv.meQuerySubdeviceByType(0, 0, MeDriver.ME_TYPE_AI, MeDriver.ME_SUBTYPE_STREAMING);
			MeRange range = new MeRange(MeDriver.ME_UNIT_VOLT, -10.0, 10.0);
			drv.meQueryRangeByMinMax(0, ns, 0, range);

			MeIOStreamConfig[] configList = new MeIOStreamConfig[2];
			for(int i = 0; i < 2; i++){
					configList[i] = new MeIOStreamConfig(
							i,
							range.getRange(),
							MeDriver.ME_REF_AI_GROUND,
							MeDriver.ME_IO_STREAM_CONFIG_TYPE_NO_FLAGS);
			}

			MeTicks acqStartTicks = new MeTicks();
			acqStartTicks.setTime(0.0);
			drv.meIOStreamTimeToTicks(
					0,
					ns,
					MeDriver.ME_TIMER_ACQ_START,
					acqStartTicks,
					MeDriver.ME_IO_STREAM_TIME_TO_TICKS_NO_FLAGS);

			MeTicks convStartTicks = new MeTicks();
			convStartTicks.setTime(0.001);
			drv.meIOStreamTimeToTicks(
					0,
					ns,
					MeDriver.ME_TIMER_CONV_START,
					convStartTicks,
					MeDriver.ME_IO_STREAM_TIME_TO_TICKS_NO_FLAGS);

			MeIOStreamTrigger trigger = new MeIOStreamTrigger(
					MeDriver.ME_TRIG_TYPE_SW,
					0,
					MeDriver.ME_TRIG_CHAN_DEFAULT,
					acqStartTicks.getTicks(),
					acqArgs,
					MeDriver.ME_TRIG_TYPE_FOLLOW,
					0,
					scanArgs,
					MeDriver.ME_TRIG_TYPE_TIMER,
					convStartTicks.getTicks(),
					convArgs,
					MeDriver.ME_TRIG_TYPE_NONE,
					0,
					MeDriver.ME_TRIG_TYPE_COUNT,
					10000,
					MeDriver.ME_IO_STREAM_TRIGGER_TYPE_NO_FLAGS);

			drv.meIOStreamConfig(
					0,
					ns,
					configList,
					trigger,
					0,
					MeDriver.ME_IO_STREAM_CONFIG_NO_FLAGS);

			MeIOStreamStart[] startList = { new MeIOStreamStart(0, ns, MeDriver.ME_START_MODE_BLOCKING, 0, MeDriver.ME_IO_STREAM_START_TYPE_NO_FLAGS) };
			drv.meIOStreamStart(startList, MeDriver.ME_IO_STREAM_START_NO_FLAGS);

			int[] values = drv.meIOStreamRead(0, ns, MeDriver.ME_READ_MODE_BLOCKING, 20000, MeDriver.ME_IO_STREAM_READ_NO_FLAGS);
			System.out.println(values.length + " values read from analog input subdevice.");

			drv.meClose(0);
		}
		catch(java.io.IOException e){
			e.printStackTrace(System.err);
		}
	}
}
