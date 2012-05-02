package examples;

import de.meilhaus.medriver.*;

public class SingleAI {
	public static void main(String[] args){
		MeDriver drv = new MeDriver();

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
			System.out.println("Search for first analog input device.");

			int ns = drv.meQuerySubdeviceByType(0, 0, MeDriver.ME_TYPE_AI, MeDriver.ME_SUBTYPE_ANY);
			MeRange range = new MeRange(MeDriver.ME_UNIT_VOLT, -10.0, 10.0);
			drv.meQueryRangeByMinMax(0, ns, 0, range);

			drv.meIOSingleConfig(
					0,
					ns,
					0,
					range.getRange(),
					MeDriver.ME_REF_AI_GROUND,
					MeDriver.ME_TRIG_CHAN_DEFAULT,
					MeDriver.ME_TRIG_TYPE_SW,
					MeDriver.ME_TRIG_EDGE_ANY,
					MeDriver.ME_IO_SINGLE_CONFIG_NO_FLAGS);

			MeIOSingle singleEntry = new MeIOSingle(0, ns, 0, MeDriver.ME_DIR_INPUT, 0, 0, MeDriver.ME_IO_SINGLE_TYPE_NO_FLAGS);
			MeIOSingle[] singleList = { singleEntry };
			drv.meIOSingle(singleList, MeDriver.ME_IO_SINGLE_NO_FLAGS);

			double physical = drv.meUtilityDigitalToPhysical(
					range.getMin(),
					range.getMax(),
					range.getMaxData(),
					singleEntry.getValue(),
					MeDriver.ME_MODULE_TYPE_MULTISIG_NONE,
					0.0);
			System.out.println("Physical value = " + physical + ".");

			drv.meClose(0);
		}
		catch(java.io.IOException e){
			e.printStackTrace(System.err);
		}
	}
}
