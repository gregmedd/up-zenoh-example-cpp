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

#include <chrono>
#include <csignal>
#include <spdlog/spdlog.h>
#include <up-client-zenoh-cpp/transport/ZenohUTransport.h>
#include <up-cpp/datamodel/builder/Uuid.h>
#include <up-cpp/client/Publisher.h>

#include "common.h"

bool gTerminate = false;

void signalHandler(int signal) {
	if (signal == SIGINT) {
		spdlog::info("Ctrl+C received. Exiting...");
		gTerminate = true;
	}
}

std::chrono::milliseconds getTime() {
	auto currentTime = std::chrono::system_clock::now();
	auto duration = currentTime.time_since_epoch();
	auto timeMilli = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

	return timeMilli;
}

uprotocol::v1::UUID getRandom() {
	return uprotocol::datamodel::builder::UuidBulder::getBuilder().build();
}

uprotocol::v1::UPayload getCounter() {
	static std::uint8_t counter = 0;
	uprotocol::v1::UPayload payload;

	using Format = uprotocol::v1::PayloadFormat::UPayloadFormat;

	payload.set_format(Format::UPAYLOAD_FORMAT_RAW);
	std::string data(1, counter++);
	payload.set_data(std::move(data));

	return payload;
}

/* The sample pub applications demonstrates how to send data using uTransport -
 * There are three topics that are published - random number, current time and a counter */
int main() {

	signal(SIGINT, signalHandler);

	auto transport = uprotocol::transport::getTransport<ZenohUTransport>(
			defaultSubscriberUri(), zenohConfig);

	if (nullptr == transport) {
		spdlog::error("ZenohUTransport init failed");
		return -1;
	}

	using namespace uprotocol::client;

	Publisher timePub(transport, timeUri(), uprotocol::v1::UPriority::UPRIORITY_CS5, 25ms);
	Publisher randPub(transport, randUri(), {}, 250ms);
	Publisher countPub(transport, countUri());
	Publisher nullPub(transport, countUri(), uprotocol::v1::UPriority::UPRIORITY_CS3);

	std::array publishers{
		[&timePub](){ return timePub.publish<TimeAsPayload>(getTime()); },
		[&randPub](){ return randPub.publish(getTime()); },
		[&countPub](){ return countPub.publish(getCounter()); },

	while (!gTerminate) {
		std::this_thread::sleep_for(1s);

		for (auto& publisher : publishers) {
			auto result = publisher();
			if (result.code() != uprotocol::v1::UCode::OK) {
				spdlog::error("Encountered error publishing message");
			}
		}
	}

	return 0;
}
