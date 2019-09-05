import yaml
import TaggingUtils

def generate_soc_ranges(soc_file, range_file, policy_inits, policy_meta):
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
    tag_map = {}
    for device_type in policy_soc:
        for elem in policy_soc[device_type]:
            name = "SOC.{}.{}".format(device_type, elem)
            if name in soc_ranges:
                tag_names = list(meta['name'] for meta in policy_soc[device_type][elem]['metadata'])
                tag_map[name] = tag_names

    meta_names = list(meta['name'] for meta in policy_meta['Metadata'])
    meta_ids = list(meta['id'] for meta in policy_meta['Metadata'])
    meta_id_map = dict(zip(meta_names, meta_ids))

    for name in tag_map:
        tag_map[name] = list(meta_id_map[tag_name] for tag_name in tag_map[name])
        tag_map[name] = list(str(tag_id) for tag_id in tag_map[name])
        tag_map[name] = "{}".format(",".join(tag_map[name]))
        range_file.write_range(soc_ranges[name][0], soc_ranges[name][1], tag_map[name])
