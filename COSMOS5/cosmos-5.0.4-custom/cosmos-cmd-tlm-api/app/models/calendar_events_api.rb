# encoding: ascii-8bit

# Copyright 2022 Ball Aerospace & Technologies Corp.
# All Rights Reserved.
#
# This program is free software; you can modify and/or redistribute it
# under the terms of the GNU Affero General Public License
# as published by the Free Software Foundation; version 3 with
# attribution addendums as found in the LICENSE.txt
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# This program may also be used under the terms of a commercial or
# enterprise edition license of COSMOS if purchased from the
# copyright holder

require_relative 'topics_thread'
require 'cosmos/utilities/authorization'

class CalendarEventsApi
  include Cosmos::Authorization

  def initialize(uuid, channel, history_count = 0, scope:, token:)
    authorize(permission: 'system', scope: scope, token: token)
    topics = ["#{scope}__cosmos_calendar"] # MUST be equal to `CalendarTopic::PRIMARY_KEY`
    @thread = TopicsThread.new(topics, channel, history_count)
    @thread.start
  end

  def kill
    @thread.stop
  end
end