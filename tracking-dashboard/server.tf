provider "oci" {}

resource "oci_core_instance" "generated_oci_core_instance" {
	agent_config {
		is_management_disabled = "false"
		is_monitoring_disabled = "false"
		plugins_config {
			desired_state = "DISABLED"
			name = "Vulnerability Scanning"
		}
		plugins_config {
			desired_state = "ENABLED"
			name = "Compute Instance Monitoring"
		}
		plugins_config {
			desired_state = "ENABLED"
			name = "Bastion"
		}
	}
	availability_config {
		recovery_action = "RESTORE_INSTANCE"
	}
	availability_domain = ""
	compartment_id = ""
	create_vnic_details {
		assign_private_dns_record = "true"
		assign_public_ip = "true"
		hostname_label = "server"
		subnet_id = ""
	}
	display_name = "Server"
	instance_options {
		are_legacy_imds_endpoints_disabled = "false"
	}
	metadata = {
		"ssh_authorized_keys" = "ssh-rsa key oracle_cloud"
	}
	shape = "VM.Standard.A1.Flex"
	shape_config {
		memory_in_gbs = "8"
		ocpus = "2"
	}
	source_details {
		boot_volume_size_in_gbs = "50"
		boot_volume_vpus_per_gb = "30"
		source_id = ""
		source_type = "image"
	}
}
