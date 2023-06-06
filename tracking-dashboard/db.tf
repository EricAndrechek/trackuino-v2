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
	availability_domain = "aTOh:US-ASHBURN-AD-3"
	compartment_id = "ocid1.tenancy.oc1..aaaaaaaafck4wwdrj3yt5fcblhhxd4u2y3npcdcvm2f2gf36e655zukilkeq"
	create_vnic_details {
		assign_private_dns_record = "true"
		assign_public_ip = "true"
		hostname_label = "db"
		subnet_id = "ocid1.subnet.oc1.iad.aaaaaaaa5lf4rv2fdgzoa2vj65ydniwxnvfyfusyh5cwa4fyjmzxrl55vnhq"
	}
	display_name = "DB"
	instance_options {
		are_legacy_imds_endpoints_disabled = "false"
	}
	metadata = {
		"ssh_authorized_keys" = "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQCvtItu9WQmzwOSnzAQZFYsZCObJmQDMeNakYflMcAgvtUYrGEw85OeKme8TD9ujrTfQ+CHxYbL+6ygE784qKQJuxrgVflTHMY19FRH1cJtTvuHRFjHt/KZyUvSRZvM9No711wpWnyQrNKxYOXEnili14IMLzmb+qXKZjkr2XpS+Ga+hmPEESCFzcOeivAYV/9l+H//GZUtRmZdQA8BxICvyqr987ICXZpDCIY0if8gb+B0WqUVbXCndwNAUwOhzk2OWcC43EKnOcuV2aGjWjPI6zkc5Q2KpLTFAYI+K2XT9ih8lr3hMDrNRktQ5+Q64lH9InCsxCSxKDMiSdKORGeP oracle_cloud"
	}
	shape = "VM.Standard.A1.Flex"
	shape_config {
		memory_in_gbs = "16"
		ocpus = "2"
	}
	source_details {
		boot_volume_size_in_gbs = "150"
		boot_volume_vpus_per_gb = "30"
		source_id = "ocid1.image.oc1.iad.aaaaaaaanetmukvnxijbewnfuw5xqtjfrm5nocv7yy4zy4ayafxolwt435bq"
		source_type = "image"
	}
}
