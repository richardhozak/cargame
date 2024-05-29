use godot::prelude::*;

#[derive(GodotClass)]
#[class(base=RefCounted)]
pub struct ServerInfo {
    #[export]
    pub fullname: GString,
    #[export]
    pub hostname: GString,
    #[export]
    pub addresses: Array<Gd<ServerAddress>>,
    #[export]
    pub properties: Dictionary,

    base: Base<RefCounted>,
}

#[godot_api]
impl IRefCounted for ServerInfo {
    fn init(base: Base<RefCounted>) -> Self {
        Self {
            fullname: GString::new(),
            hostname: GString::new(),
            addresses: Array::new(),
            properties: Dictionary::new(),
            base,
        }
    }

    fn to_string(&self) -> GString {
        let Self {
            hostname,
            addresses,
            properties,
            ..
        } = &self;

        format!("ServerInfo(hostname={hostname}, addresses={addresses}, properties={properties})")
            .into()
    }
}

#[derive(GodotClass)]
#[class(no_init)]
pub struct ServerAddress {
    #[export]
    ip_type: IpType,
    #[export]
    ip: GString,
    #[export]
    port: u16,
}

#[godot_api]
impl ServerAddress {
    #[func]
    pub fn from_ip_port(ip_type: IpType, ip: GString, port: u16) -> Gd<Self> {
        Gd::from_object(Self { ip_type, ip, port })
    }
}

#[godot_api]
impl IRefCounted for ServerAddress {
    fn to_string(&self) -> GString {
        let Self { ip, port, .. } = &self;

        format!("ServerAddress(ip={ip}, port={port})").into()
    }
}

#[derive(GodotConvert, Var, Export, Debug)]
#[godot(via = GString)]
pub enum IpType {
    V4,
    V6,
}
