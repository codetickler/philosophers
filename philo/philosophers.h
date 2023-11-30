/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philosophers.h                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gude-cas <gude-cas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/25 13:18:46 by gude-cas          #+#    #+#             */
/*   Updated: 2023/11/25 15:12:26 by gude-cas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILOSOPHERS_H
#define PHILOSOPHERS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

typedef struct s_data
{
    int number_of_philosophers;
    int time_to_die;
    int time_to_eat;
    int time_to_sleep;
    int number_of_times_each_philosopher_must_eat;
    int print;
    int die_status;
    unsigned long timestamp;
    struct s_philo *philos;
    pthread_t checker;
    pthread_mutex_t *forks;
    pthread_mutex_t die_mutex;
    pthread_mutex_t print_mutex;
    pthread_mutex_t time_to_eat_mutex;
    pthread_mutex_t time_to_die_mutex;
}   t_data;

typedef struct s_philo
{
    int id;
    int meals;
    int last_meal;
    int died;
    struct s_data *data;
    pthread_t thread;
    pthread_mutex_t *left_fork;
    pthread_mutex_t *right_fork;
    pthread_mutex_t meals_mutex;
    pthread_mutex_t last_meal_mutex;
    pthread_mutex_t die_mutex;
}   t_philo;

int ascii_to_int(char *str);
int check_args(int ac, char **av);

#endif