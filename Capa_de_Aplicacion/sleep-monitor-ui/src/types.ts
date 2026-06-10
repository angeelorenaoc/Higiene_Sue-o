export interface Reading {
  id: number;
  id_reading_type: number;
  value: number;
  created_at: string;
}

export interface Rule {
  id: number;
  id_reading_type: number;
  id_condition_type: number;
  id_actuator_type: number;
  condition_value: number;
  created_at: string;
}

export interface ActuatorLog {
  id: number;
  id_actuator_type: number;
  id_rule: number;
  id_reading: number;
  command: string;
  created_at: string;
}

export interface Lookup {
  id: number;
  name: string;
}