/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utility.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nallani <nallani@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/09 21:16:08 by nallani           #+#    #+#             */
/*   Updated: 2023/02/02 10:18:34 by nallani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef UTILITY_H
# define UTILITY_H

#include <memory>
#include <string>
#include <stdexcept>
#include <fstream>

template <typename ... Args>
std::string string_format( const std::string& format, Args ... args );

#include "Utility.tpp"
#endif
