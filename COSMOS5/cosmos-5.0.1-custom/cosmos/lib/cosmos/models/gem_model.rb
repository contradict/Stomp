# encoding: ascii-8bit

# Copyright 2021 Ball Aerospace & Technologies Corp.
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

require 'open-uri'
require 'nokogiri'
require 'httpclient'
require 'cosmos/utilities/s3'

module Cosmos
  # This class acts like a Model but doesn't inherit from Model because it doesn't
  # actual interact with the Store (Redis). Instead we implement names, get, put
  # and destroy to allow interaction with gem files from the PluginModel and
  # the GemsController.
  class GemModel
    @@bucket_initialized = false

    def self.names
      rubys3_client = initialize_bucket()
      gems = []
      rubys3_client.list_objects(bucket: 'gems').contents.each do |object|
        gems << object.key
      end
      gems
    end

    def self.get(dir, name)
      rubys3_client = initialize_bucket()
      path = File.join(dir, name)
      rubys3_client.get_object(bucket: 'gems', key: name, response_target: path)
      return path
    end

    def self.put(gem_file_path)
      rubys3_client = initialize_bucket()
      if File.file?(gem_file_path)
        gem_filename = File.basename(gem_file_path)
        Logger.info "Installing gem: #{gem_filename}"
        File.open(gem_file_path, 'rb') do |file|
          rubys3_client.put_object(bucket: 'gems', key: gem_filename, body: file)
        end
      else
        message = "Gem file #{gem_file_path} does not exist!"
        Logger.error message
        raise message
      end
    end

    def self.destroy(name)
      rubys3_client = initialize_bucket()
      Logger.info "Removing gem: #{name}"
      rubys3_client.delete_object(bucket: 'gems', key: name)
    end

    # private

    def self.initialize_bucket
      rubys3_client = Aws::S3::Client.new
      unless @@bucket_initialized
        begin
          rubys3_client.head_bucket(bucket: 'gems')
        rescue Aws::S3::Errors::NotFound
          rubys3_client.create_bucket(bucket: 'gems')
        end
        @@bucket_initialized = true
      end
      return rubys3_client
    end
  end
end
