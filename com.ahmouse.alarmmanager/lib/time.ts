import { Temporal } from "@js-temporal/polyfill";

export function createTimeObject(hours: number, minutes: number) {
    return new Temporal.PlainTime(hours, minutes);
}