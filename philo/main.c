/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gude-cas <gude-cas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/25 13:25:44 by gude-cas          #+#    #+#             */
/*   Updated: 2023/11/27 16:36:22 by gude-cas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

/* returns the current time in milliseconds */
unsigned long get_time(void)
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return(time.tv_sec * 1000 + time.tv_usec / 1000);
}

/* sleeps a number of milliseconds using 'usleep', with 
   some additional logic to ensure accurate timing */
void sleeper(int ms)
{
    unsigned long start = get_time();
    usleep(ms * 950);
    while(get_time() - start < (unsigned long)ms)
        usleep(10);
}

/* returns the time elapsed since the program started in milliseconds */
unsigned long get_current_time(void)
{
    static unsigned long start;
    if(!start)
        start = get_time();
    return(get_time() - start);
}

/* prints the elapsed time, the philos ID and the provided message */
void write_message(t_philo *philo, char *str)
{
    unsigned long time;
    if(!str)
        return ;
    pthread_mutex_lock(&philo->data->print_mutex);
    if(philo->data->print)
    {
        time = get_current_time() - philo->data->timestamp;
        printf("\033[1;3m%lu\033[0m \033[1;37m%d\033[0m %s\n", time, philo->id, str);
    }
    pthread_mutex_unlock(&philo->data->print_mutex);
}

/* initializes mutexes for the forks used by the philos */
int init_forks_loop(t_data *info)
{
    int i = 0;
    while(i < info->number_of_philosophers)
    {
        if(pthread_mutex_init(&info->forks[i], NULL) != 0)
            return(1);
        i++;
    }
    return(0);
}

/* allocates memory for an array of mutexes representing forks and initializes them */
pthread_mutex_t *init_forks(t_data *info)
{
    if(!info)
        return(NULL);
    info->forks = malloc(sizeof(pthread_mutex_t) * info->number_of_philosophers);
    if(!info->forks)
        return(NULL);
    if(init_forks_loop(info) != 0)
        return(NULL);
    return(info->forks);
}

/* initializes the 't_data' struct with information such as the number of 
   philosophers, time constraints and mutexes */
t_data *init_info(char **av)
{
    t_data *info = malloc(sizeof(t_data));
    if(!info)
        return(NULL);
    info->number_of_philosophers = ascii_to_int(av[1]);
    info->time_to_die = ascii_to_int(av[2]);
    info->time_to_eat = ascii_to_int(av[3]);
    info->time_to_sleep = ascii_to_int(av[4]);
    info->number_of_times_each_philosopher_must_eat = ascii_to_int(av[5]);
    if(!info->number_of_philosophers || !info->time_to_die || !info->time_to_eat || !info->time_to_sleep)
    {
        printf("Invalid arguments.\n");
        return(NULL);
    }
    info->print = 1;
    info->die_status = 0;
    pthread_mutex_init(&info->die_mutex, NULL);
    pthread_mutex_init(&info->print_mutex, NULL);
    pthread_mutex_init(&info->time_to_die_mutex, NULL);
    pthread_mutex_init(&info->time_to_eat_mutex, NULL);
    info->timestamp = get_current_time();
    init_forks(info);
    return(info);
}

/* initializes a philosophers data, including ID, meal-related variables, fork 
   pointers and mutexes */
void init_philo(t_data *info, int id)
{
    info->philos[id].id = id + 1;
    info->philos[id].data = info;
    info->philos[id].meals = 0;
    pthread_mutex_init(&info->philos[id].meals_mutex, NULL);
    info->philos[id].last_meal = info->timestamp;
    pthread_mutex_init(&info->philos[id].last_meal_mutex, NULL);
    info->philos[id].right_fork = &info->forks[id];
    info->philos[id].left_fork = &info->forks[(id + 1) % info->number_of_philosophers];
    info->philos[id].data = info;
    info->philos[id].died = 0;
    pthread_mutex_init(&info->philos[id].die_mutex, NULL);
}

/* initializes all philos in the 't_data' struct */
void init_philos_loop(t_data *info)
{
    int i = 0;
    while(i < info->number_of_philosophers)
    {
        init_philo(info, i);
        i++;
    }
}

/* allocates memory for an array of 't_philo' structs representing each philosopher */
t_philo *malloc_philos(t_data *info)
{
    info->philos = malloc(sizeof(t_philo) * info->number_of_philosophers);
    if(!info->philos)
        return(NULL);
    return(info->philos);
}

/* initializes an array of philos in the 't_data' struct */
t_philo *init_philos(t_data *info)
{
    if(!info)
        return(NULL);
    malloc_philos(info);
    if(!info->philos)
        return(NULL);
    init_philos_loop(info);
    return(info->philos);
}

/* unlocks the mutexes for both of a philosophers forks */
void put_forks(t_philo *philo)
{
    pthread_mutex_unlock(philo->left_fork);
    pthread_mutex_unlock(philo->right_fork);
}

/* returns which fork a philo should choose considering if the philo ID is even or odd */
pthread_mutex_t *choose_fork(t_philo *philo, int order)
{
    if(philo->id % 2 == 0)
    {
        if(order % 2 == 0)
            return(philo->left_fork);
        return(philo->right_fork);
    }
    else
    {
        if(order % 2 == 0)
            return(philo->right_fork);
        return(philo->left_fork);
    }
}

/* checks if a philo has died and releases forks if necessary */
int is_dead(t_philo *philo)
{
    pthread_mutex_lock(&philo->data->die_mutex);
    pthread_mutex_lock(&philo->die_mutex);
    if(philo->data->die_status || philo->died)
    {
        put_forks(philo);
        pthread_mutex_unlock(&philo->die_mutex);
        pthread_mutex_unlock(&philo->data->die_mutex);
        return(1);
    }
    pthread_mutex_unlock(&philo->die_mutex);
    pthread_mutex_unlock(&philo->data->die_mutex);
    return(0);
}

/* attempts to aquire the philosophers designated forks and writes a messaage if successfull */
void take_forks(t_philo *philo)
{
    if(is_dead(philo))
    {
        pthread_mutex_unlock(choose_fork(philo, 1));
        return ;
    }
    pthread_mutex_lock(choose_fork(philo, 1));
    write_message(philo, "\033[1;33mhas taken a fork\033[0m");
    if(is_dead(philo))
    {
        pthread_mutex_unlock(choose_fork(philo, 2));
        return ;
    }
    pthread_mutex_lock(choose_fork(philo, 2));
    write_message(philo, "\033[1;33mhas taken a fork\033[0m");
}

/* updates meal-related variables and writes a message indicating that the philo is eating */
void philo_eat(t_philo *philo)
{
    pthread_mutex_lock(&philo->last_meal_mutex);
    philo->last_meal = (int)get_current_time();
    pthread_mutex_unlock(&philo->last_meal_mutex);
    pthread_mutex_lock(&philo->meals_mutex);
    philo->meals++;
    pthread_mutex_unlock(&philo->meals_mutex);
    write_message(philo, "\033[1;36mis eating\033[0m");
    sleeper(philo->data->time_to_eat);    
}

/* writes a message indicating that a philo is sleeping */
void philo_sleep(t_philo *philo)
{
    write_message(philo, "\033[1;49mis sleeping\033[0m");
    sleeper(philo->data->time_to_sleep);
}

/* writes a message indicating that a philo is thinking */
void philo_think(t_philo *philo)
{
    write_message(philo, "\033[1;32mis thinking\033[0m");
}

/* lock mutexes for philo-specific data */
void philo_mutex_lock(t_philo *philo)
{
    pthread_mutex_lock(&philo->last_meal_mutex);
    pthread_mutex_lock(&philo->meals_mutex);    
}

/* unlock mutexes for philo-specific data */
void philo_mutex_unlock(t_philo *philo)
{
    pthread_mutex_unlock(&philo->meals_mutex);
    pthread_mutex_unlock(&philo->last_meal_mutex);    
}

/* sets the program-wide 'die_status', indicating that a philo has died */
void a_philo_has_died(t_data *info)
{
    pthread_mutex_lock(&info->print_mutex);
    pthread_mutex_lock(&info->die_mutex);
    info->print = 0;
    info->die_status = 1;
    pthread_mutex_unlock(&info->die_mutex);
    pthread_mutex_unlock(&info->print_mutex);
}

/* checks if a philo has starved to death based on the time since their last meal */
int philo_starved(t_philo * philo)
{
    philo_mutex_lock(philo);
    if(get_current_time() - philo->last_meal > (unsigned long)philo->data->time_to_die)
    {
        philo_mutex_unlock(philo);
        write_message(philo, "\033[1;31mdied\033[0m");
        a_philo_has_died(philo->data);
        return(1);
    }
    philo_mutex_unlock(philo);
    return(0);
}

/* checks if all philosophers have eaten the required number of times */
int philo_eaten_enough(t_data *info)
{
    int i = 0;
    int status = 1;
    if(!info->number_of_times_each_philosopher_must_eat)
        return(0);
    while(i < info->number_of_philosophers)
    {
        pthread_mutex_lock(&info->philos[i].meals_mutex);
        if(info->philos[i].meals < info->number_of_times_each_philosopher_must_eat)
            status = 0;
        else
        {
            pthread_mutex_lock(&info->die_mutex);
            info->philos[i].died = 1;
            pthread_mutex_unlock(&info->die_mutex);
        }
        pthread_mutex_unlock(&info->philos[i].meals_mutex);
        i++;
    }
    return(status);
}

/* a thread routine that continuosly checks the status of philosophers 
   and terminates if necessary */
void *checker(void *arg)
{
    int i = 0;
    t_data *info = (t_data *)arg;
    while(i < info->number_of_philosophers)
    {
        if(philo_starved(info->philos + i) || philo_eaten_enough(info))
            return(NULL);
        if(i == info->number_of_philosophers - 1)
            i = 0;
        else
            i++;
    }
    return(NULL);
}

/* the main routine executed by each philosopher thread, handling the cycle of taking 
   forks, eating, sleeping and thinking */
void *routine(void *arg)
{
    t_philo *philo = (t_philo *)arg;
    if(philo->data->number_of_philosophers == 1)
        sleeper(philo->data->time_to_die + 2);
    while(1)
    {
        if(is_dead(philo))
            return(NULL);
        take_forks(philo);
        if(is_dead(philo))
            return(NULL);
        philo_eat(philo);
        if(is_dead(philo))
            return(NULL);
        put_forks(philo);
        if(is_dead(philo))
            return(NULL);
        philo_sleep(philo);
        if(is_dead(philo))
            return(NULL);
        philo_think(philo);
    }
    return(NULL);
}

/* creates threads for each philosopher and a checker thread */
void create_philo_threads(t_data *info)
{
    int i = 0;
    t_philo *philos = info->philos;
    while(i < info->number_of_philosophers)
    {
        pthread_create(&philos[i].thread, NULL, routine, &philos[i]);
        i++;
    }
    pthread_create(&info->checker, NULL, checker, info);
}

/* waits for all philosopher threads and the checker thread to finish */
void join_philo_threads(t_data *info)
{
    int i = 0;
    while(i < info->number_of_philosophers)
    {
        pthread_join(info->philos[i].thread, NULL);
        i++;
    }
    pthread_join(info->checker, NULL);
    i = 0;
}

/* runs the whole program */
void run_philosophers(t_data *info)
{
    create_philo_threads(info);
    join_philo_threads(info);
}

int main(int ac, char **av)
{
    t_data *info;
    
    if(ac < 5 || ac > 6)
    {
        printf("Invalid number of arguments.\n");
        return(1);
    }
    else
    {
        if(check_args(ac, av))
            return(1);
        info = init_info(av);
        if(!info)
            return(1);
        if(!init_philos(info))
            return(1);
        run_philosophers(info);
        free(info->philos);
        free(info->forks);
        free(info);
    }
}