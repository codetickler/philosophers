/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gude-cas <gude-cas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/25 13:45:57 by gude-cas          #+#    #+#             */
/*   Updated: 2023/11/27 13:46:44 by gude-cas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

/* convert ascii to digit */
int ascii_to_int(char *str)
{
    int i = 0;
    unsigned int n = 0;
    if(!str)
        return(0);
    while(str[i] && str[i] == ' ')
        i++;
    while(str[i])
    {
        n = (str[i] - '0') + (n * 10);
        i++;
    }
    if(n == 2147483648)
        return(-1);
    return(n);
}

/* return 0 if input is an int */
int is_int(char *str)
{
    int n = ascii_to_int(str);
    if(n == -1)
        return(1);
    return(0);    
}

/* return 0 if input is a positive number */
int is_negative(char *str)
{
    while(*str && *str == ' ')
        str++;
    if(*str == '-')
        return(1);
    return(0);
}

/* return 0 if input contains only digits */
int is_digit(char *str)
{
    int i = 0;
    while(str[i])
    {
        if(str[i] < '0' || str[i] > '9')
            return(1);
        i++;
    }
    return(0);
}

/* return 1 if input arg is empty ("") */
int is_empty(char *str)
{
    if(!str || !*str)
        return(1);
    return(0);
}

/* return 0 if all checks are parsed correctly */
int check_args(int ac, char **av)
{
    while(--ac > 0)
    {
        if(is_empty(av[ac]) || is_digit(av[ac]) || is_negative(av[ac]) || is_int(av[ac]))
        {
            printf("Invalid arguments.\n");
            return(1);
        }
    }
    return(0);
}