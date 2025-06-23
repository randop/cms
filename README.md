# CMS
Content management server project written in C++ with MongoDB

<p>
  <img src="https://img.shields.io/badge/version-1.0.19-blue.svg?cacheSeconds=2592000" />
  <img src="https://img.shields.io/badge/STD-C%2B%2B20-white.svg?cacheSeconds=2592000" />
  <a href="https://github.com/randop/cms">
    <img alt="Maintenance" src="https://img.shields.io/badge/Maintained%3F-yes-green.svg" target="_blank" />
  </a>
</p>

>  Copyright © 2010 — 2025 Randolph Ledesma
>
> Licensed under the Apache License, Version 2.0 (the "License");
> you may not use this file except in compliance with the License.
> You may obtain a copy of the License at
>
>    http://www.apache.org/licenses/LICENSE-2.0
>
> Unless required by applicable law or agreed to in writing, software
> distributed under the License is distributed on an "AS IS" BASIS,
> WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
> See the License for the specific language governing permissions and
> limitations under the License.
>

Originated from [https://gitlab.com/randop/applications/](https://gitlab.com/randop/applications/)

## Dependencies
1. `libmongoc` and `libbson`
```bash
sudo mkdir -p /opt/mongo-c-driver/current
sudo git clone -b v2.0.2 --depth 1 https://github.com/mongodb/mongo-c-driver.git /opt/mongo-c-driver/2.0.2
sudo cd /opt/mongo-c-driver/2.0.2 && cmake -DCMAKE_INSTALL_PREFIX=/opt/mongo-c-driver/current .
sudo cd /opt/mongo-c-driver/2.0.2 && make all install
sudo echo "/opt/mongo-c-driver/current" > /etc/ld.so.conf.d/boost.conf
sudo ldconfig
```

## Development
> 1.) Boot the development mongo database
```bash
docker compose up --renew-anon-volumes
```
> 2.) Configure the Build
```bash
export LD_LIBRARY_PATH=/usr/local/lib:/opt/boost/current/lib:/opt/mongo-c-driver/current/lib
meson setup build --prefer-static --default-library=static
```
> 3.) Compile
```bash
meson compile -C build
```

Copyright © 2010 — 2025 [Randolph Ledesma](https://github.com/randop).

Last updated on 2025-06-23T11:02:07.000Z UTC
