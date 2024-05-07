/*
 * Copyright (c) 2024 General Motors GTO LLC
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * SPDX-FileType: SOURCE
 * SPDX-FileCopyrightText: 2024 General Motors GTO LLC
 * SPDX-License-Identifier: Apache-2.0
 */
#include <chrono>
#include <csignal>
#include <unistd.h>
#include <up-client-zenoh-cpp/transport/ZenohUTransport.h>
#include <up-cpp/client/RpcTarget.h>
#include <spdlog/spdlog.h>

#include "common.h"

bool gTerminate = false;

void signalHandler(int signal) {
	if (signal == SIGINT) {
		spdlog::info("Ctrl+C received. Exiting...");
		gTerminate = true; 
	}
}

/// @brief RPC Target serving out current time as milliseconds from epoch
struct TimeService {
	/// @brief
	using UriAsString = uprotocol::datamodel::serializer::uuri::asString;

	/// @brief
	TimeService(std::shared_ptr<uprotocol::transport::UTransport> transport)
		: target_( std::move(transport), timeServerUri(),
				std::move([this](const uprotocol::v1::UMessage& m){
					giveTime(m);
				})) { }

	/// @brief
	void giveTime(const uprotocol::v1::UMessage& request) const {
		auto now = timeSinceEpoch();
		target_.respondTo<TimeAsPayload>(request, now);
	}

private:
	/// @brief
	std::chrono::milliseconds timeSinceEpoch() {
		auto currentTime = std::chrono::system_clock::now();
		return std::chrono::duration_cast<std::chrono::milliseconds>(
				currentTime.time_since_epoch());
	}

	uprotocol::client::RpcTarget target_;
};

// This sample RPC server demonstrates how to receive RPC requests and send a
// response back to the client. In this example, the current time represented
// as milliseconds since epoch.
int main() {
	signal(SIGINT, signalHandler);

	auto transport = uprotocol::transport::getTransport<ZenohUTransport>(
			defaultServerSource(), zenohCfgPath);

	if (nullptr == transport) {
		spdlog::error("ZenohUTransport init failed");
		return -1;
	}

	TimeService tserv(transport);

	while (!gTerminate) {
		sleep(1);
	}

	return 0;
}
