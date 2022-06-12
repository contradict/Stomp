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

require 'cosmos/models/trigger_model'
require 'cosmos/topics/autonomic_topic'

class TriggerController < ApplicationController
  def initialize
    @model_class = Cosmos::TriggerModel
  end

  # Returns an array/list of trigger values in json.
  #
  # group [String] the group name, `system`
  # scope [String] the scope of the trigger, `TEST`
  # @return [String] the array of triggers converted into json format
  def index
    begin
      authorize(permission: 'system', scope: params[:scope], token: request.headers['HTTP_AUTHORIZATION'])
    rescue Cosmos::AuthError => e
      render(:json => { :status => 'error', :message => e.message }, :status => 401) and return
    rescue Cosmos::ForbiddenError => e
      render(:json => { :status => 'error', :message => e.message }, :status => 403) and return
    end
    begin
      ret = Array.new
      triggers = @model_class.all(group: params[:group], scope: params[:scope])
      triggers.each do |_, trigger|
        ret << trigger
      end
      render :json => ret, :status => 200
    rescue StandardError => e
      render :json => { :status => 'error', :message => e.message, 'type' => e.class, 'backtrace' => e.backtrace }, :status => 500
    end
  end

  # Returns a single trigger in json.
  #
  # group [String] the group name, `systemGroup`
  # name [String] the trigger name, `TV1-12345`
  # scope [String] the scope of the trigger, `TEST`
  # @return [String] the array of triggers converted into json format.
  def show
    begin
      authorize(permission: 'system', scope: params[:scope], token: request.headers['HTTP_AUTHORIZATION'])
    rescue Cosmos::AuthError => e
      render(:json => { :status => 'error', :message => e.message }, :status => 401) and return
    rescue Cosmos::ForbiddenError => e
      render(:json => { :status => 'error', :message => e.message }, :status => 403) and return
    end
    begin
      model = @model_class.get(name: params[:name], group: params[:group], scope: params[:scope])
      if model.nil?
        render :json => { :status => 'error', :message => 'not found' }, :status => 404
        return
      end
      render :json => model.as_json(), :status => 200
    rescue Cosmos::TriggerInputError => e
      render :json => { :status => 'error', :message => e.message, 'type' => e.class }, :status => 400
    rescue StandardError => e
      render :json => { :status => 'error', :message => e.message, 'type' => e.class, 'backtrace' => e.backtrace }, :status => 500
    end
  end

  # Create a new trigger and return the object/hash of the trigger in json.
  #
  # group [String] the group name, `systemGroup`
  # name [String] the trigger name, `TV1-12345`
  # scope [String] the scope of the trigger, `TEST`
  # json [String] The json of the event (see #trigger_model)
  # @return [String] the trigger converted into json format
  # Request Headers
  #```json
  #  {
  #    "Authorization": "token/password",
  #    "Content-Type": "application/json"
  #  }
  #```
  # Request Post Body
  #```json
  #  {
  #    "description": "POSX > 690000",
  #    "group": "mango",
  #    "left": {
  #      "type": "item",
  #      "item": "POSX",
  #    },
  #    "operator": ">",
  #    "right": {
  #      "type": "value",
  #      "value": 690000,
  #    }
  #  }
  #```
  def create
    begin
      authorize(permission: 'run_script', scope: params[:scope], token: request.headers['HTTP_AUTHORIZATION'])
    rescue Cosmos::AuthError => e
      render(:json => { :status => 'error', :message => e.message }, :status => 401) and return
    rescue Cosmos::ForbiddenError => e
      render(:json => { :status => 'error', :message => e.message }, :status => 403) and return
    end
    hash = nil
    begin
      hash = params.to_unsafe_h.slice(:group, :left, :operator, :right).to_h
      name = @model_class.create_mini_id()
      model = @model_class.from_json(hash.symbolize_keys, name: name, scope: params[:scope])
      model.create()
      render :json => model.as_json, :status => 201
    rescue Cosmos::TriggerInputError => e
      render :json => { :status => 'error', :message => e.message, 'type' => e.class }, :status => 400
    rescue Cosmos::TriggerError => e
      render :json => { :status => 'error', :message => e.message, 'type' => e.class }, :status => 418
    rescue StandardError => e
      render :json => { :status => 'error', :message => e.message, 'type' => e.class, 'backtrace' => e.backtrace }, :status => 500
    end
  end

  # Set reaction active to true
  #
  # group [String] the group name, `systemGroup`
  # name [String] the trigger name, `TV1-12345`
  # scope [String] the scope of the reaction, `TEST`
  # @return [String] the trigger as a object/hash converted into json format
  # Request Headers
  #```json
  #  {
  #    "Authorization": "token/password",
  #    "Content-Type": "application/json"
  #  }
  #```
  # Request Post Body
  #```json
  #  {}
  #```
  def activate
    begin
      authorize(permission: 'run_script', scope: params[:scope], token: request.headers['HTTP_AUTHORIZATION'])
    rescue Cosmos::AuthError => e
      render(:json => { :status => 'error', :message => e.message }, :status => 401) and return
    rescue Cosmos::ForbiddenError => e
      render(:json => { :status => 'error', :message => e.message }, :status => 403) and return
    end
    begin
      model = @model_class.get(name: params[:name], group: params[:group], scope: params[:scope])
      if model.nil?
        render :json => { :status => 'error', :message => 'not found' }, :status => 404
        return
      end
      model.activate() if model.active == false
      render :json => model.as_json, :status => 200
    rescue StandardError => e
      render :json => { :status => 'error', :message => e.message, 'type' => e.class, 'backtrace' => e.backtrace }, :status => 500
    end
  end

  # Set reaction active to false
  #
  # group [String] the group name, `systemGroup`
  # name [String] the trigger name, `TV1-1234`
  # scope [String] the scope of the reaction, `TEST`
  # @return [String] the trigger as a object/hash converted into json format
  # Request Headers
  #```json
  #  {
  #    "Authorization": "token/password",
  #    "Content-Type": "application/json"
  #  }
  #```
  # Request Post Body
  #```json
  #  {}
  #```
  def deactivate
    begin
      authorize(permission: 'run_script', scope: params[:scope], token: request.headers['HTTP_AUTHORIZATION'])
    rescue Cosmos::AuthError => e
      render(:json => { :status => 'error', :message => e.message }, :status => 401) and return
    rescue Cosmos::ForbiddenError => e
      render(:json => { :status => 'error', :message => e.message }, :status => 403) and return
    end
    begin
      model = @model_class.get(name: params[:name], group: params[:group], scope: params[:scope])
      if model.nil?
        render :json => { :status => 'error', :message => 'not found' }, :status => 404
        return
      end
      model.deactivate() if model.active == true
      render :json => model.as_json, :status => 200
    rescue StandardError => e
      render :json => { :status => 'error', :message => e.message, 'type' => e.class, 'backtrace' => e.backtrace }, :status => 500
    end
  end

  # Removes an trigger trigger by score/id.
  #
  # group [String] the group name, `systemGroup`
  # name [String] the trigger name, `TV1-12345`
  # scope [String] the scope of the trigger, `TEST`
  # @return [String] object/hash converted into json format but with a 204 no-content status code
  # Request Headers
  #```json
  #  {
  #    "Authorization": "token/password",
  #    "Content-Type": "application/json"
  #  }
  #```
  def destroy
    begin
      authorize(permission: 'run_script', scope: params[:scope], token: request.headers['HTTP_AUTHORIZATION'])
    rescue Cosmos::AuthError => e
      render(:json => { :status => 'error', :message => e.message }, :status => 401) and return
    rescue Cosmos::ForbiddenError => e
      render(:json => { :status => 'error', :message => e.message }, :status => 403) and return
    end
    begin
      @model_class.delete(name: params[:name], group: params[:group], scope: params[:scope])
      render :json => {}, :status => 204
    rescue Cosmos::TriggerInputError => e
      render :json => { :status => 'error', :message => e.message, 'type' => e.class }, :status => 404
    rescue Cosmos::TriggerError => e
      render :json => { :status => 'error', :message => e.message, 'type' => e.class }, :status => 400
    rescue StandardError => e
      render :json => { :status => 'error', :message => e.message, 'type' => e.class, 'backtrace' => e.backtrace }, :status => 500
    end
  end

end
