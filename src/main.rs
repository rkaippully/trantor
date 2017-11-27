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
pub extern fn kmain() {
    let p = 0xb8000 as *mut u16;
    unsafe {
        *p = 0x4752;
    }
    loop {}
}
