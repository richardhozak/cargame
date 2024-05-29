use godot::prelude::*;

mod server_browser;
mod server_info;

struct ServerBrowserExtension;

#[gdextension]
unsafe impl ExtensionLibrary for ServerBrowserExtension {}
