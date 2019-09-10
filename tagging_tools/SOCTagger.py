import yaml
import TaggingUtils

def generate_soc_ranges(soc_file, range_file, policy_inits):
    soc_cfg = yaml.load(soc_file.read(), Loader=yaml.FullLoader)

    soc_ranges = {}
    for elem in soc_cfg['SOC']:
        elem_fields = soc_cfg['SOC'][elem]
        name = elem_fields['name']
        start = elem_fields['start']
        end = elem_fields['end']
        soc_ranges[name] = (start, end)

    # filter SOC range entries by those required by the policy init
    policy_soc = policy_inits['Require']['SOC']
    for device_type in policy_soc:
        for elem in policy_soc[device_type]:
            name = "SOC.{}.{}".format(device_type, elem)
            if name in soc_ranges:
                print('{0}: 0x{1:X} - 0x{2:X}'.format(name, soc_ranges[name][0], soc_ranges[name][1]))
                range_file.write_range(soc_ranges[name][0], soc_ranges[name][1], name)
