#!/bin/bash
component="$1"
package_prefix="cfengine-nova"

if [ -z "${CFEngine_Enterprise_Package_Version}" ]; then
  package_version="3.21.2"
else
  package_version="${CFEngine_Enterprise_Package_Version}"
fi

if [ -z "${CFEngine_Enterprise_Package_Release}" ]; then
  package_release="1"
else
  package_release="${CFEngine_Enterprise_Package_Release}"
fi

package_source="https://cfengine-package-repos.s3.amazonaws.com/enterprise/Enterprise-$package_version"

function prepare_deps {
  if [ -e "/etc/debian_version" ]; then
    dpkg -l | grep -q wget || apt-get -y install wget
    return
  fi
  if [ -e "/etc/redhat-release" ]; then
    rpm -qa | grep -q wget || yum -y install wget
    return
  fi
  if [ -e "/etc/SuSE-release" ]; then
    rpm -qa | grep -q wget || zypper --non-interactive install wget
    return
  fi
}


function usage {
    echo "$0 [hub|agent]"
    exit 1
}

if [[ $# != 1 ]]; then
    echo "Missing required argument"
    usage
fi

DISTRO=""  # Ubuntu|Debian|RedHatEnterprise*|SUSE|CentOS
RELEASE="" # digit
uname_arch=""  # x86_64|i386
lsb_release=$(which lsb_release)

if [ -z "$TEST" ]; then
    run_prefix=""
else
    run_prefix="/bin/echo "
fi

function detect_distro {
  if [ -e "$lsb_release" ]; then
      DISTRO=$($lsb_release --short --id)
      RELEASE=$($lsb_release --short --release)
      return
  fi
  if [ -e "/etc/os-release" ]; then
      source /etc/os-release
      if [ "$ID" == "rhel" ]; then
          DISTRO="RedHatEnterpriseLinux"
          RELEASE="$VERSION_ID"
          return
      fi
      if [ "$ID" == "centos" ]; then
          DISTRO="CentOS"
          RELEASE="$VERSION_ID"
          return
      fi
      if [ "$ID" == "amzn" ]; then
          DISTRO="AmazonLinux"
          RELEASE="$VERSION_ID"
          return
      fi
      if [ "$ID" == "sles" ]; then
          DISTRO="SUSE"
          RELEASE="$VERSION_ID"
          return
      fi
      # typically Ubuntu will have lsb_release except in the case of the standard docker images which will land in this /etc/os-release handling section
      if [ "$ID" == "ubuntu" ]; then
          DISTRO="Ubuntu"
          RELEASE="$VERSION_ID"
          return
      fi
      if [ "$ID" == "ol" ]; then
          DISTRO="OracleLinux"
          RELEASE="$VERSION_ID"
          return
      fi
  elif [ -e "/etc/redhat-release" ]; then
      if grep "CentOS" /etc/redhat-release; then
          DISTRO="$(awk '/release/ {print $1}' /etc/redhat-release)"
          RELEASE="$(awk '/release/ {print $3}' /etc/redhat-release)"
          return
      fi
      if grep "Red Hat" /etc/redhat-release; then
          DISTRO="RedHatEnterpriseServer"
          RELEASE="$(awk '/release/ {print $7}' /etc/redhat-release)"
          return
      fi
  elif [ -e "/etc/SuSE-release" ]; then
      # SLES 12+ has /etc/os-release and we use that
      if grep "Enterprise Server 11" /etc/SuSE-release; then
          DISTRO="SUSE"
          RELEASE=$(awk '/VERSION/ {print $3}' /etc/SuSE-release).$(awk '/PATCHLEVEL/ {print $3}' /etc/SuSE-release)
          return
      fi
  fi
  if [ -e "/etc/debian_version" ]; then
      DISTRO="Debian"
      RELEASE=$(cat /etc/debian_version)
      return
  fi


  echo "Sorry I don't know what distro this is. We recommend using cf-remote https://github.com/cfengine/cf-remote."
  exit 1
}

function get_package_arch {
# Determine the architecture name used in the package name
if [ $uname_arch == "unknown" ]; then
    uname_arch=$(uname -m)
fi
case $1 in
    deb)
        # Enterprise and community deb packages use different naming conventions
        # enterprise hub uses x86_64 and community and enterprise clients use amd64
        if [ $uname_arch == "x86_64" ]; then
              package_arch="amd64"
        elif [ $uname_arch == "aarch64" ]; then
          package_arch="arm64"
          folder_arch_suffix="arm_64"
        else
          package_arch="i386"
        fi
        ;;

    rpm)
        if [ $uname_arch == "x86_64" ]; then
          package_arch="$uname_arch"
        else
          package_arch="i386"
        fi
        ;;
    *)
        echo "Sorry I don't know about this platforms package architecture. We recommend using cf-remote https://github.com/cfengine/cf-remote."
        ;;
esac
}

function construct_enterprise_package_name_deb {
  if [ "$component" == "hub" ]; then
    package_name="$package_prefix"-hub_"$package_version"-"$package_release"."$comp_tag"_"$package_arch".deb
  elif [ "$component" == "agent" ]; then
    package_name="$package_prefix"_"$package_version"-"$package_release"."$comp_tag"_"$package_arch".deb
  else
      echo "Unknown component: $component"
      usage
  fi
}

function construct_enterprise_package_name_rpm {
  if [ "$component" == "hub" ]; then
    package_name="$package_prefix"-hub-"$package_version"-"$package_release"."$comp_tag"."$package_arch".rpm
  elif [ "$component" == "agent" ]; then
    package_name="$package_prefix"-"$package_version"-"$package_release"."$comp_tag"."$package_arch".rpm
  else
      echo "Unknown component: $component"
      usage
  fi
}

function construct_package_name {
case $DISTRO in
    Ubuntu|Debian)
        get_package_arch deb
        construct_enterprise_package_name_deb
        ;;
    RedHatEnterprise*|SUSE|CentOS|AmazonLinux|OracleLinux)
        get_package_arch rpm
        construct_enterprise_package_name_rpm
        ;;
    *)
        echo "Sorry I don't know the package name format on this system. We recommend using cf-remote https://github.com/cfengine/cf-remote."
        exit 1
        ;;
esac
}

function get_enterprise_hub_package_dir {
if [ "$uname_arch" != "x86_64" ]; then
  echo "Sorry, enterprise hubs require a 64 bit machine. We recommend using cf-remote https://github.com/cfengine/cf-remote."
  exit 1
fi

case $DISTRO in
    Ubuntu)
        MAJOR_RELEASE=$(echo $RELEASE | cut -d "." -f 1)
        if [ "$MAJOR_RELEASE" -eq 22 ]; then
            # Ubuntu 22
            folder="ubuntu_22_x86_64"
            comp_tag="ubuntu22"
        elif [ "$MAJOR_RELEASE" -eq 20 ]; then
            # Ubuntu 20
            folder="ubuntu_20_x86_64"
            comp_tag="ubuntu20"
        elif [ "$MAJOR_RELEASE" -eq 18 ]; then
            # Ubuntu 18
            folder="ubuntu_18_x86_64"
            comp_tag="ubuntu18"
        elif [ "$MAJOR_RELEASE" -eq 16 ]; then
            # Ubuntu 16
            folder="ubuntu_16_x86_64"
            comp_tag="ubuntu16"
        elif [ "$MAJOR_RELEASE" -gt 22 ]; then
            echo "Sorry, we do not officially support hub functionality on $DISTRO $MAJOR_RELEASE."
            echo "Installing the package that is most likely to work. We recommend using cf-remote https://github.com/cfengine/cf-remote."
            # Ubuntu >20
            folder="ubuntu_20_x86_64"
            comp_tag="ubuntu20"
        else
            echo "Sorry, we do not support hub functionality on $DISTRO $MAJOR_RELEASE."
            echo "Please try a supported release. We recommend using cf-remote https://github.com/cfengine/cf-remote."
            exit 1
        fi
        ;;
    Debian)
        MAJOR_RELEASE=$(echo $RELEASE | cut -d "." -f 1)
        if [ "$MAJOR_RELEASE" -lt 7 ]; then
            echo "Sorry, we do not support hub functionality on $DISTRO $MAJOR_RELEASE."
            echo "Please try upgrading to a newer release. We recommend using cf-remote https://github.com/cfengine/cf-remote."
            exit 1
        elif [ "$MAJOR_RELEASE" -eq 7 ]; then
            folder="debian_7_x86_64"
            comp_tag="debian7"
            echo "Sorry, we do not officially support hub functionality on $DISTRO $MAJOR_RELEASE."
            echo "Installing the package that is most likely to work. We recommend using cf-remote https://github.com/cfengine/cf-remote."
        elif [ "$MAJOR_RELEASE" -eq 8 ]; then
            folder="debian_8_x86_64"
            comp_tag="debian8"
        elif [ "$MAJOR_RELEASE" -eq 9 ]; then
            folder="debian_9_x86_64"
            comp_tag="debian9"
        elif [ "$MAJOR_RELEASE" -eq 10 ]; then
            folder="debian_10_x86_64"
            comp_tag="debian10"
        elif [ "$MAJOR_RELEASE" -gt 10 ]; then
            folder="debian_10_x86_64"
            comp_tag="debian10"
            echo "Sorry, we do not officially support hub functionality on $DISTRO $MAJOR_RELEASE."
            echo "Installing the package that is most likely to work. We recommend using cf-remote https://github.com/cfengine/cf-remote."
        else
            echo "Sorry, we do not currently support hub functionality on $DISTRO $MAJOR_RELEASE."
            echo "Please try a supported release. We recommend using cf-remote https://github.com/cfengine/cf-remote."
            exit 1
        fi
        ;;
    RedHatEnterprise*|CentOS|OracleLinux)
        MAJOR_RELEASE=$(echo $RELEASE | cut -d "." -f 1)
        if [ "$MAJOR_RELEASE" -lt 6 ]; then
            echo "Sorry, we do not support hub functionality on your version of $DISTRO."
            echo "Please try upgrading to a newer release. We recommend using cf-remote https://github.com/cfengine/cf-remote."
            exit 1
        elif [ "$MAJOR_RELEASE" -eq 6 ]; then
            folder="redhat_6_x86_64"
            comp_tag="el6"
        elif [ "$MAJOR_RELEASE" -eq 7 ]; then
            folder="redhat_7_x86_64"
            comp_tag="el7"
        elif [ "$MAJOR_RELEASE" -eq 8 ]; then
            folder="redhat_8_x86_64"
            comp_tag="el8"
        elif [ "$MAJOR_RELEASE" -eq 9 ]; then
            folder="redhat_9_x86_64"
            comp_tag="el9"
        else
            echo "Sorry, we do not support hub functionality on your version of $DISTRO."
            echo "Please try a supported release. We recommend using cf-remote https://github.com/cfengine/cf-remote."
            exit 1
        fi
        ;;
    AmazonLinux)
        MAJOR_RELEASE=$(echo $RELEASE | cut -d "." -f 1)
        if [ "$MAJOR_RELEASE" -eq 2016 ]; then
            folder="redhat_6_x86_64"
            comp_tag="el6"
        elif [ "$MAJOR_RELEASE" -gt 2016 ]; then
            folder="redhat_6_x86_64"
            comp_tag="el6"
        elif [ "$MAJOR_RELEASE" -eq 2 ]; then
            folder="redhat_7_x86_64"
            comp_tag="el7"
        else
            echo "Sorry, we do not support hub functionality on your version of $DISTRO."
            echo "Please try a supported release. We recommend using cf-remote https://github.com/cfengine/cf-remote."
            exit 1
        fi
        ;;
    SUSE)
        echo "Error: SUSE is not supported as an Enterprise hub platform. We recommend using cf-remote https://github.com/cfengine/cf-remote."
        exit 1
        ;;
    *)
        echo "Sorry I dont know how to fetch the $component package for $DISTRO $RELEASE. We recommend using cf-remote https://github.com/cfengine/cf-remote."
        exit 1
esac
}

function get_enterprise_client_package_dir_and_tag {
case $DISTRO in
    Ubuntu)
        MAJOR_RELEASE=$(echo $RELEASE | cut -d "." -f 1)
        if [[ "$uname_arch" != "x86_64" ]] && [[ "$uname_arch" != "aarch64" ]]; then
            # All 32bit packages use the debian 7 compatible build
            folder="agent_deb_i386"
            comp_tag="debian7"
        elif [[ "$uname_arch" == "aarch64" ]] && [[ $MAJOR_RELEASE -eq 22 ]]; then
            folder_arch_suffix="arm_64"
            folder="agent_ubuntu${MAJOR_RELEASE}_${folder_arch_suffix}"
            comp_tag="ubuntu${MAJOR_RELEASE}"
        elif [ "$MAJOR_RELEASE" -ge "22" ]; then
            # Ubuntu 22+
            folder="agent_ubuntu22_x86_64"
            comp_tag="ubuntu22"
        elif [ "$MAJOR_RELEASE" -ge "20" ]; then
            # Ubuntu 20+
            folder="agent_ubuntu20_x86_64"
            comp_tag="ubuntu20"
        elif [ "$MAJOR_RELEASE" -ge "18" ]; then
            # Ubuntu 18-19
            folder="agent_ubuntu18_x86_64"
            comp_tag="ubuntu18"
        elif [ "$MAJOR_RELEASE" -ge "16" ]; then
            # Ubuntu 16-17
            folder="agent_ubuntu16_x86_64"
            comp_tag="ubuntu16"
        else
            echo "Sorry $DISTRO $MAJOR_RELEASE is not supported. We recommend using cf-remote https://github.com/cfengine/cf-remote."
        fi
        ;;
    Debian)
        MAJOR_RELEASE=$(echo $RELEASE | cut -d "." -f 1)
        if [ "$uname_arch" != "x86_64" ]] && [[ "$uname_arch" != "aarch64" ]]; then
            # All 32bit packages use the Debian 7 compatible build
            folder="agent_deb_i386"
            comp_tag="debian7"
        elif [[ "$uname_arch" == "aarch64" ]] && [[ "$MAJOR_RELEASE" -ge "11" ]]; then
            folder_arch_suffix="arm_64"
            folder="agent_debian${MAJOR_RELEASE}_${folder_arch_suffix}"
            comp_tag="debian${MAJOR_RELEASE}"
        elif [ "$MAJOR_RELEASE" -lt 7 ]; then
            # Debian <7
            folder="agent_deb_x86_64"
            comp_tag="debian7"
        elif [ "$MAJOR_RELEASE" -eq 7 ]; then
            # Debian 7
            folder="agent_deb_x86_64"
            comp_tag="debian7"
        elif [ "$MAJOR_RELEASE" -eq 8 ]; then
            # Debian 8
            folder="agent_debian8_x86_64"
            comp_tag="debian8"
        elif [ "$MAJOR_RELEASE" -eq 9 ]; then
            # Debian 9
            folder="agent_debian9_x86_64"
            comp_tag="debian9"
        elif [ "$MAJOR_RELEASE" -eq 10 ]; then
            # Debian 10
            folder="agent_debian10_x86_64"
            comp_tag="debian10"
        elif [ "$MAJOR_RELEASE" -gt 10 ]; then
            # Debian 10+
            folder="agent_debian10_x86_64"
            comp_tag="debian10"
        else
            echo "Sorry $DISTRO $MAJOR_RELEASE is not supported. We recommend using cf-remote https://github.com/cfengine/cf-remote."
        fi
        ;;
    RedHatEnterprise*|SUSE|CentOS|AmazonLinux|OracleLinux)
        # Red hat
        MAJOR_RELEASE=$(echo $RELEASE | cut -d "." -f 1)
        # Red Hat, CentOS, Oracle Linux, all follow the same release numbers
        if [[ "$DISTRO" = RedHatEnterprise* || "$DISTRO" = CentOS || "$DISTRO" = OracleLinux ]]; then
            if [ "$MAJOR_RELEASE" -ge 9 ]; then
                folder="agent_rhel9_x86_64"
                comp_tag="el9"
            elif [ "$MAJOR_RELEASE" -ge 8 ]; then
                folder="agent_rhel8_x86_64"
                comp_tag="el8"
            elif [ "$MAJOR_RELEASE" = 7 ]; then
                folder="agent_rhel7_x86_64"
                comp_tag="el7"
            elif [ "$MAJOR_RELEASE" = 6 ]; then
                folder="agent_rhel6_x86_64"
                comp_tag="el6"
            fi
        # Amazon Linux follows it's own release numbers
        elif [[ "$DISTRO" = "AmazonLinux" ]] && [ "$MAJOR_RELEASE" = 2 ]; then
                folder="agent_rhel7_x86_64"
                comp_tag="el7"
        # SUSE follows it's own release numbers
        elif [ "$DISTRO" == "SUSE" ] && [ "$MAJOR_RELEASE" -gt 10 ]; then
            folder="agent_rhel6_x86_64"
            comp_tag="el6"
        else
            folder="agent_rpm_$uname_arch"
            comp_tag="el5"
        fi
        ;;
     *)
         echo "Sorry I dont know how to fetch the $component package for $DISTRO $RELEASE. We recommend using cf-remote https://github.com/cfengine/cf-remote."
        exit 1
       ;;
esac
}

function fetch_the_package {
  $run_prefix wget $package_source/$component/$folder/$package_name
}

function thanks {
echo "Ready to bootstrap using /var/cfengine/bin/cf-agent --bootstrap <ip>"
}

function install_package {
case $DISTRO in
    Ubuntu|Debian)
        $run_prefix dpkg -i $package_name && thanks
        ;;
    RedHatEnterprise*|SUSE|CentOS|AmazonLinux|OracleLinux)
        $run_prefix rpm -i $package_name && thanks
        ;;
    *)
        echo "Sorry I don't know how to install $package_name on $DISTRO $RELEASE. We recommend using cf-remote https://github.com/cfengine/cf-remote."
        exit 1
esac
}

function cleanup {
    $run_prefix rm -f $package_name
}

prepare_deps
if [ -z "${CFEngine_Enterprise_TEST_Distro}" -o -z "${CFEngine_Enterprise_TEST_Release}" ]; then
  detect_distro
else
  DISTRO="${CFEngine_Enterprise_TEST_Distro}"
  RELEASE="${CFEngine_Enterprise_TEST_Release}"
fi
if [ -z "${CFEngine_Enterprise_TEST_Arch}" ]; then
  uname_arch=$(uname -m)
else
  uname_arch="${CFEngine_Enterprise_TEST_Arch}"
fi
if [ "$component" == "hub" ]; then
  get_enterprise_hub_package_dir
else
  get_enterprise_client_package_dir_and_tag
fi
construct_package_name
fetch_the_package
install_package
cleanup
