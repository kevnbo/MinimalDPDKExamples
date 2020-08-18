terraform {
  required_version = ">= 0.12"
}

provider aws {
  region = var.aws_region
  profile = "default"
}

resource "aws_placement_group" "thing" {
  name = "thingies"
  # clusster | partition | spread
  # cluster puts us in the highest bisection segment
  strategy = "cluster"
}

resource "aws_subnet" "mgmt" {
   vpc_id = var.vpc_id
   cidr_block = var.mgmt_subnet_cidr
   tags = {
     Name = "Mgmt"
   }
}

resource "aws_subnet" "traffic" {
   vpc_id = var.vpc_id
   cidr_block = var.traffic_subnet_cidr
   tags = {
     Name = "Traffic"
   }
}

resource "aws_security_group" "mgmt" {
  name = "mgmt"
  description = "sg for mgmt"
  vpc_id = var.vpc_id
  ingress {
    from_port = 0
	to_port = 22
	protocol = "TCP"
	cidr_blocks = ["0.0.0.0/0"]
  }

  egress {
    from_port = 0
    to_port = 0
    protocol = "-1"
    cidr_blocks = ["0.0.0.0/0"]
  }
}

resource "aws_security_group" "traffic" {
  name = "traffic"
  description = "sg for traffic"
  vpc_id = var.vpc_id
  ingress {
    from_port = 0
	to_port = 65535
	protocol = "UDP"
	cidr_blocks = ["0.0.0.0/0"]
  }
  ingress {
    from_port = 0
	to_port = 65535
	protocol = "TCP"
	cidr_blocks = ["0.0.0.0/0"]
  }
  ingress {
    from_port = -1
	to_port = -1
	protocol = "ICMP"
	cidr_blocks = ["0.0.0.0/0"]
  }

  egress {
    from_port = 0
    to_port = 0
    protocol = "-1"
    cidr_blocks = ["0.0.0.0/0"]
  }
}

resource "aws_instance" "thing" {
  connection {
    type = "ssh"
    user = "ec2-user"
	host = self.public_ip
  }
  instance_type = "c5n.xlarge"
  #instance_type = "t2.micro"
  ami = var.aws_amis[var.aws_region]
  key_name = var.aws_key_pair
  vpc_security_group_ids = [aws_security_group.mgmt.id]
  subnet_id = aws_subnet.mgmt.id
#  placement_group = aws_placement_group.thing.id

  associate_public_ip_address = true
  source_dest_check = false

  tags = {
  	   Name = "thing1"
  }
  provisioner "local-exec" {
   command = "echo yoop did a thing"
  }
}

resource "aws_instance" "left" {
  instance_type = "c5n.xlarge"
# instance_type = "t3.medium"
  ami = var.aws_amis[var.aws_region]
  key_name = var.aws_key_pair
  vpc_security_group_ids = [aws_security_group.mgmt.id]
  subnet_id = aws_subnet.mgmt.id
  placement_group = aws_placement_group.thing.id

  associate_public_ip_address = true
  source_dest_check = false

  tags = {
  	   Name = "left"
  }
  provisioner "local-exec" {
   command = "echo yoop did a thing > /tmp/thingy.txt"
  }
}

resource "aws_instance" "right" {
 instance_type = "c5n.xlarge"
#  instance_type = "t3.medium"
  ami = var.aws_amis[var.aws_region]
  key_name = var.aws_key_pair
  vpc_security_group_ids = [aws_security_group.mgmt.id]
  subnet_id = aws_subnet.mgmt.id
  placement_group = aws_placement_group.thing.id

  associate_public_ip_address = true
  source_dest_check = false

  tags = {
  	   Name = "right"
  }
  provisioner "local-exec" {
   command = "echo yoop did a thing > /tmp/thingy.txt"
  }
}

resource "aws_network_interface" "thingleft" {
  subnet_id = aws_subnet.traffic.id
  security_groups = [aws_security_group.traffic.id]
  description = "traffic to the left"
  attachment {
    instance = aws_instance.thing.id
	device_index = 1
  }
  tags = {
     Name = "thingL"
  }
}

resource "aws_network_interface" "thingright" {
  subnet_id = aws_subnet.traffic.id
  security_groups = [aws_security_group.traffic.id]
  description = "traffic to the right"
  attachment {
    instance = aws_instance.thing.id
	device_index = 2
  }
  tags = {
     Name = "thingR"
  }
}

resource "aws_network_interface" "left" {
  subnet_id = aws_subnet.traffic.id
  security_groups = [aws_security_group.traffic.id]
  description = "gen left"
  attachment {
    instance = aws_instance.left.id
	device_index = 1
  }
  tags = {
     Name = "left"
  }
}

resource "aws_network_interface" "right" {
  subnet_id = aws_subnet.traffic.id
  security_groups = [aws_security_group.traffic.id]
  description = "gen right"
  attachment {
    instance = aws_instance.right.id
	device_index = 1
  }
  tags = {
     Name = "right"
  }
}

