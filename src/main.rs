// main.rs
#![allow(warnings)]
use libc::c_int;
use std::ffi::CString;
// mod add;
mod nvencdec;

fn decode_media(
    cu_device: c_int,
    media_uri: &str,
    output_format: nvencdec::OutputFormat,
    out_file_path: &str,
) {
    // Convert strings to C-style strings (CString)
    let c_media_uri = CString::new(media_uri).expect("CString::new failed");
    let c_out_file_path = CString::new(out_file_path).expect("CString::new failed");

    unsafe {
        // Call the C function via the generated bindings
        nvencdec::DecodeProc(
            cu_device,
            c_media_uri.as_ptr(),
            output_format,
            c_out_file_path.as_ptr(),
        );
    }
}

fn main() {
    let device_id: i32 = 0; // Change this according to your use case
    let media_uri = "/home/satyam/dev/nv_encdec/input.mp4";
    let output_format = nvencdec::OutputFormat_bgra64; // Use an appropriate format from your bindings
    let output_file_path = "/home/satyam/dev/nv_encdec/output";

    decode_media(device_id, media_uri, output_format, output_file_path);
}
//
// fn main() {
//     unsafe {
//         // Call the C function 'add'
//         let a: ::std::os::raw::c_int = 3;
//         let b: ::std::os::raw::c_int = 3;
//         let result: c_int = add::add(a, b);
//         println!("Result of add(3, 4): {}", result);
//     }
// }
