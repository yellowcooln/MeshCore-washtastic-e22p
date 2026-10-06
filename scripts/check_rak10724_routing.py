#!/usr/bin/env python3
"""Source-level routing guard; this does not claim over-air validation."""
from pathlib import Path
root = Path(__file__).resolve().parents[1]
s = (root / 'examples/simple_repeater/MyMesh.cpp').read_text()
manual = s.split('void MyMesh::sendSelfAdvertisement(', 1)[1].split('void MyMesh::updateAdvertTimer', 1)[0]
assert 'sendBatteryInfoAdvert' in manual.split('} else {', 1)[0], 'Manual flood reporting missing'
assert 'sendBatteryInfoAdvert' not in manual.split('} else {', 1)[1], 'Local advert must not report'
loop = s.split('void MyMesh::loop()', 1)[1]
flood, local = loop.split('} else if (next_local_advert', 1)
assert 'sendBatteryInfoAdvert' in flood, 'Scheduled flood reporting missing'
assert 'sendBatteryInfoAdvert' not in local, 'Scheduled local advert must not report'
assert 'sendFloodScoped(default_scope, pkt' in manual and 'sendFloodScoped(default_scope, pkt' in flood
assert s.count('sendBatteryInfoAdvert(delay_millis);') == 2
print('PASS: manual/scheduled flood hooks; local/zero-hop negative paths; scoped adverts')
