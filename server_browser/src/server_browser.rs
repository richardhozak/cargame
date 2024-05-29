use godot::{engine::notify::NodeNotification, prelude::*};
use mdns_sd::{Receiver, ServiceDaemon, ServiceEvent, ServiceInfo};

use crate::server_info::{IpType, ServerAddress, ServerInfo};

#[derive(GodotClass)]
#[class(tool, base=Node)]
pub struct ServerBrowser {
    #[export]
    #[var(get, set = set_service_name)]
    service_name: GString,
    service: ServiceDaemon,
    service_events: Option<Receiver<ServiceEvent>>,
    service_fullname: Option<String>,
    #[var]
    server_list: Array<Gd<ServerInfo>>,
    base: Base<Node>,
}

#[godot_api]
impl ServerBrowser {
    #[signal]
    fn server_list_changed();

    #[func]
    pub fn set_service_name(&mut self, value: GString) {
        self.service_name = value;
        self.base_mut().update_configuration_warnings();
    }

    #[func]
    pub fn browse(&mut self) {
        godot_print!("Browse servers for {}", self.service_name);

        let service_type = format!("_{}._udp.local.", self.service_name);
        let browse = self
            .service
            .browse(&service_type)
            .expect("Failed to browse");

        self.service_events = Some(browse);
    }

    #[func]
    pub fn register(&mut self, port: u16) {
        godot_print!("Registering service {} on port {}", self.service_name, port);

        let service_type = format!("_{}._udp.local.", self.service_name);
        let instance_name = format!("{}", std::process::id());
        let service_hostname = format!("{}.local.", gethostname::gethostname().to_string_lossy());

        godot_print!("Service hostname {}", service_hostname);

        let service_info = ServiceInfo::new(
            &service_type,
            &instance_name,
            &service_hostname,
            "",
            port,
            None,
        )
        .expect("valid service info")
        .enable_addr_auto();

        let service_fullname = service_info.get_fullname().to_string();

        self.service
            .register(service_info)
            .expect("Failed to register mDNS service");

        godot_print!("Registered service {}", service_fullname);

        self.service_fullname = Some(service_fullname);
    }
}

#[godot_api]
impl INode for ServerBrowser {
    fn init(base: Base<Node>) -> Self {
        godot_print!("Server Browser Init");

        Self {
            service_name: GString::new(),
            service: ServiceDaemon::new().expect("Could not create mDNS service daemon"),
            service_events: None,
            service_fullname: None,
            server_list: Array::new(),
            base,
        }
    }

    fn get_configuration_warnings(&self) -> PackedStringArray {
        return validate_service_name(&self.service_name.to_string());
    }

    fn process(&mut self, _delta: f64) {
        let mut changed = false;
        if let Some(receiver) = &self.service_events {
            for event in receiver.try_iter() {
                match event {
                    ServiceEvent::ServiceResolved(info) => {
                        let mut server_info = if let Some(server_info) = self
                            .server_list
                            .iter_shared()
                            .find(|server| server.bind().fullname == info.get_fullname())
                        {
                            server_info
                        } else {
                            let mut server_info = ServerInfo::new_gd();
                            server_info.bind_mut().fullname = info.get_fullname().to_string();
                            self.server_list.push(server_info);
                            self.server_list.back().unwrap()
                        };

                        let mut server_info = server_info.bind_mut();

                        let hostname = info.get_hostname();
                        let hostname = info
                            .get_hostname()
                            .strip_suffix(".local.")
                            .unwrap_or(hostname);

                        server_info.hostname = hostname.into();
                        server_info.addresses =
                            Array::from_iter(info.get_addresses().iter().map(|ip| {
                                ServerAddress::from_ip_port(
                                    if ip.is_ipv4() { IpType::V4 } else { IpType::V6 },
                                    ip.to_string().to_godot(),
                                    info.get_port(),
                                )
                            }));

                        changed = true;
                    }
                    ServiceEvent::ServiceRemoved(_, fullname) => {
                        let index = self.server_list.iter_shared().enumerate().find_map(
                            |(index, server_info)| {
                                if server_info.bind().fullname == fullname {
                                    Some(index)
                                } else {
                                    None
                                }
                            },
                        );

                        if let Some(index) = index {
                            self.server_list.remove(index);
                            changed = true;
                        }
                    }
                    _ => {}
                }
            }
        }

        if changed {
            self.base_mut()
                .emit_signal("server_list_changed".into(), &[]);
        }
    }

    fn on_notification(&mut self, what: NodeNotification) {
        if what == NodeNotification::PREDELETE {
            if let Some(service_fullname) = &self.service_fullname {
                self.service
                    .unregister(service_fullname)
                    .expect("Could not unregister mDNS service");
            }
        }
    }
}

fn validate_service_name(name: &str) -> PackedStringArray {
    let mut errors = PackedStringArray::new();

    if name.starts_with('-') {
        errors.push("Service name cannot start with hyphen (\"-\")".into());
    }

    if name.ends_with('-') {
        errors.push("Service name cannot end with hyphen (\"-\")".into());
    }

    if name.contains("--") {
        errors.push("Service name cannot contain double hyphens (\"--\")".into());
    }

    if name.len() > 15 {
        errors.push("Service name cannot be longer than 15 characters".into());
    }

    if name.is_empty() {
        errors.push("Service name cannot be empty".into());
    }

    if !name.bytes().all(|c| c.is_ascii_alphanumeric() || c == b'-') {
        errors.push("Service name can only contain a-zA-Z and hyphens ('-')".into());
    }

    if !name.bytes().any(|c| c.is_ascii_alphabetic()) {
        errors.push("Service name must contains at least one letter (a-zA-Z)".into());
    }

    errors
}
