// Copyright (c) 2024 General Motors GTO LLC
//
// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.
// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: 2024 General Motors GTO LLC
// SPDX-License-Identifier: Apache-2.0

#ifndef RPC_COMMON_H
#define RPC_COMMON_H

#include <up-core-api/uri.pb.h>
#include <up-cpp/uri/builder/BuildUUri.h>
#include <up-cpp/uri/builder/BuildUAuthority.h>
#include <up-cpp/uri/builder/BuildEntity.h>
#include <up-cpp/uri/builder/BuildUResource.h>

uprotocol::v1::UUri timeServerUri() {
	UUri timeSrv;
	timeSrv.set_ue_id(1);
	timeSrv.set_ue_version_major(1);
	timeSrv.set_resource_id(1);
	return timeSrv;
}

uprotocol::v1::UUri defaultServerUri() {
	UUri server;
	server.set_ue_id(1);
	server.set_ue_version_major(1);
	return server;
}

uprotocol::v1::UUri defaultClientUri() {
	UUri client;
	client.set_ue_id(2);
	client.set_ue_version_major(1);
	return client;
}

/// @brief Serializer for millisecond time representations
struct TimeAsPayload {
	static uprotocol::v1::UPayload serialize(std::chrono::milliseconds t) {
		uprotocol::v1::UPayload payload;
		payload.format(uprotocol::v1::UPayloadFormat::UPAYLOAD_FORMAT_RAW);
		std::string data;
		auto millisec = t.count();
		data.resize(sizeof(millisec));
		*(reinterpret_cast<decltype(millisec)*>(data.data())) = millisec;
		payload.data(std::move(data));
		return payload;
	}

	static std::chrono::milliseconds deserialize(const uprotocol::v1::UPayload& p) {
		if (p.format() != uprotocol::v1::UPayloadFormat::UPAYLOAD_FORMAT_RAW) {
			throw std::invalid_argument("Cannot deserialize time from non-raw payload");
		}
		if (p.data().size() < sizeof(std::chrono::milliseconds::rep)) {
			throw std::invalid_argument("Size of payload smaller than milliseconds::rep");
		}

		auto millisec = *(reinterpert_cast<
				std::chrono::milliseconds::rep*>(p.data().data()));

		return std::chrono::milliseconds(millisec);
	}
};


#endif // RPC_COMMON_H
