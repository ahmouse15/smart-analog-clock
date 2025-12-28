import { Temporal } from "@js-temporal/polyfill";

export enum dayOfWeek {
  Monday,
  Tuesday,
  Wednesday,
  Thursday,
  Friday,
  Saturday,
  Sunday
};

export type Alarm = {
  id: string,
  time: Temporal.PlainTime,
  enabled: boolean,
  recurringDays: dayOfWeek[] | null,
};