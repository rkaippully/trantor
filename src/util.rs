/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  This Source Code Form is "Incompatible With Secondary Licenses", as
  defined by the Mozilla Public License, v. 2.0.
*/

#[no_mangle]
pub extern fn to_bcd(x: u8) -> u16 {
  let mut low: u16 = x as u16 & 0x0f;
  low += if low < 10 { '0' as u16 } else { 'a' as u16 - 10 };
  let mut high: u16 = (x as u16 & 0xf0) >> 4;
  high += if high < 10 { '0' as u16 } else { 'a' as u16 - 10 };
  (high << 8) | low
}
