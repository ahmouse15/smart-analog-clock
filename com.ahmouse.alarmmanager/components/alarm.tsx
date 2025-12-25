import * as React from 'react';
import { Switch, Text} from 'react-native-paper';
import { StyleSheet, View } from 'react-native';

type AlarmListItemProps = {
  time: Date
};

export function AlarmListItem(props: AlarmListItemProps) {
  // Extract hours and minutes in locale-specific manner
  let timeStr = [props.time.getHours(), props.time.getMinutes()].join(':');

  return (
    <View style={styles.container}>
      <View style={styles.leftContainer}>
        <Text variant="displayLarge">{timeStr}</Text>
        <Text variant="titleMedium">Schedule</Text>
      </View>
      <View style={styles.rightContainer}>
        <Switch/>
      </View>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    height: "auto",
    width: '100%',
    backgroundColor: 'none',
    alignItems: 'center',
    flexDirection: 'row',
    justifyContent: 'space-between',
    padding: 10,
    borderWidth: 1,
    borderColor: "blue"
  },
  leftContainer: {
    borderWidth: 1,
    borderColor: "red"
  },
  rightContainer: {
    borderWidth: 1,
    borderColor: "green"

  }
});
