variable ssh_ip_cidr {
  description = "ip cidr to allow ssh access on mgmt subnet"
}

variable aws_key_pair {
  description = "keypair to use"
}

variable "aws_region" {
  description = "AWS region name"
  default = "us-east-1"
}

variable "vpc_id" {
  description = "The id of the VPC the mgmt+traffic subnets will be created in"
  default = "vpc-0ce6b7e144cafd843"
}

variable traffic_subnet_cidr {
  description = "a /24 cidr within the vpc for traffic subnet, e.g., 10.28.14.0/24"
  default = "10.28.28.0/24"
}

variable mgmt_subnet_cidr {
  description = "a /24 cidr within the vpc for mgmt subnet, e.g., 10.28.1.0/24"
  default = "10.28.2.0/24"
}

# amazon linux ami
variable "aws_amis" {
  default = {
  		  us-east-1 = "ami-02354e95b39ca8dec"
  }
}
